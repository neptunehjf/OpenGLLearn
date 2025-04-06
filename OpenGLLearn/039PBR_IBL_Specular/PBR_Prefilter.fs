#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1;// 環境cubemap
uniform float roughness;//　粗さ
uniform float resolution;// 環境cubemap解像度

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint num);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);

void main()
{
    // 因为镜面光与视角方向有关，而预计算无法知道视角方向。
    // 折衷的方案是假设宏观上V,N,I在同一条直线上

    // 鏡面反射は視点方向に依存するが事前計算時には未知のため
    // マクロ的V,N,Iが同一線上にあるという仮定で近似

    // 这里片段的宏观法线，正好等于局部坐标
    // フラグメントのマクロ法線（局部座標と等価）
    vec3 N = normalize(localPos);    
    
    // 宏观入射向量
    // マクロ入射ベクトル
    vec3 I = N;                       
    
    // 宏观反射向量
    // マクロ反射ベクトル
    vec3 V = N;                      
    
    vec3 prefilterColor = vec3(0.0);
    const uint samplesNum = 1024u;
    float totalWeight = 0.0;

    for (uint i = 0u; i < samplesNum; i++)
    {
        // 生成二维随机采样点
        // 2次元低差異列生成
        vec2 Xi = Hammersley(i, samplesNum);             
        // 生成微观表面的半程向量，通过重要性采样，主要集中于宏观表面的法线N附近
        // マイクロ表面のハーフベクトル生成（法線N近傍に集中する重要度サンプリング）
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // 根据宏观的反射方向V 和 微观的H，算出微观的入射方向L，即用于采样环境cubemap的向量
        // マクロ反射方向VとマイクロHからマイクロ入射方向Lを算出（環境マップサンプリング用）
        vec3 L = normalize(reflect(-V, H)); 

        // 由于H是重要性采样得到的，算出的L集中在宏观入射角I附近，这里再次根据方差多次加权，可以提高离I近的颜色的权重
        // Hの重要度に基づく重み付け
        float weight = max(dot(L, I), 0.0);

        // 算出当前采样的H的pdf
        // 現在サンプリング中のHベクトルのPDFを計算
        float D = DistributionGGX(N, H, roughness); // 法線分布率(pdf)
        float NdotH = max(dot(N, H), 0.0);          // NとHの角度に反比例
        float HdotV = max(dot(H, V), 0.0);          // VとHの角度に反比例

        // unreal engine4 的pdf计算公式，不是很理解是怎么来的，下面说一下自己的理解
        // pdf在D的基础上计算，NdotH(N,H夹角)额外影响了pdf，N,H夹角越小，pdf越大，但这在DistributionGGX里已经体现了，不知道为什么这里还要考虑
        // HdotV(V,H夹角)额外影响了pdf，这个可能是考虑了Fresnel方程，V与H夹角越大，反射越强
        // pdf越大和反射越强是等价的，因为重要性采样的样本集中在能让反射最强的地方
        // 另外由于V == I == N，这里实际就是 float pdf = D / 4.0 + 0.0001; 以下公式是为了适配更普适的情况
        //
        // Unreal Engine 4のPDF計算式に関する技術的考察
        // （理論的背景の解釈メモ：導出過程が不明なため独自分析）
        // ◆ 式の構成要素分析
        // PDF = D * NdotH / (4.0 * HdotV) の意味:
        // - 基盤となるD（GGX法線分布関数）に追加項を乗算
        // - NdotH項：NとHの角度が小さい程PDF↑（GGX内で既に考慮されている特性の重複　理解できない）
        // - HdotV項：VとHの角度が大きい程PDF↑（フレネル効果の反射確率上昇を補正）
        // V == I == Nの場合、float pdf = D / 4.0 + 0.0001と等価
        //float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;  

        // 直接 用D作为pdf，更符合我自己的理解，而且效果也一样
        // 直接DをPDFとして使用する方が、私自身の理解に沿っており、結果も同等です。
        float pdf = D + 0.0001;  

        float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(samplesNum) * pdf + 0.0001);

        // 根据pdf算出mipLevel，注意算出的小数会在textureLod自动向下取整
        // 这个公式暂时也不知道怎么来的。以下是自己的理解
        // saSample是符合pdf分布的样本数的倒数，样本数越大，采样越精确，mipLevel越小
        // saTexel是单位像素对应的采样立体角，采样立体角越小，所需样本就越多。样本一定的情况下，mipLevel越大
        //
        // PDFに基づいてMIPレベルを算出。小数点以下の値はtextureLodで自動的に切り捨てられる
        // 現時点ではこの式の理論的根拠を把握していません。以下は独自解釈です
        // saSampleはPDF分布に従うサンプル数の逆数。サンプル数↑ → サンプリング精度↑ → mipLevel↓
        // saTexelは単位ピクセルが対応するサンプリング立体角。立体角↓ → 必要サンプル数↑ → サンプル数一定の場合mipLevel↑
        float mipLevel = (roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel)); 
        //float mipLevel = 0.0; 

        prefilterColor += (textureLod(texture_cubemap1, L, mipLevel).rgb * weight);
        totalWeight += weight;
    }

    // 加重平均を求める
    prefilterColor = prefilterColor / totalWeight;

    FragColor = vec4(prefilterColor, 1.0);
}


// Van Der Corput 序列 生成的仍然是随机样本，但样本分布更均匀，返回值是归一化到0到1之间的小数
// Van Der Corput 列: 生成されるサンプルはランダムだが、分布がより均一化される
// 戻り値は0～1範囲に正規化された小数値
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------

// Hammersley 以Van Der Corput 序列为基础，构造了一组二维随机样本
// Hammersley列: Van Der Corput列を基に二次元ランダムサンプル群を構築
vec2 Hammersley(uint i, uint num)
{
    return vec2(float(i)/float(num), RadicalInverse_VdC(i));
}

// 重要性采样，根据Hammersley二维随机样本得出一个符合目标pdf（主要集中在N附近，并受到roughness影响）的H向量
// 重要度サンプリング：Hammersley二次元ランダムサンプルから目標PDF（主にN近傍に集中、粗さ影響有）に合致するHベクトルを生成
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    // unreal engine4 用的是粗糙度的平方，效果更好
    // Unreal Engine4では粗さの二乗を使用（結果最適化のため）
    float a = roughness * roughness;

    // 从Hammersley分布得到对应的Tangent空间的球面坐标
    // phi由Xi.x(Hammersley采样点)得到，与概率无关
    // Hammersley分布から接空間の球面座標を計算
    // phi: Xi.xに比例（確率密度非依存）
    float phi = 2 * PI * Xi.x;

    // theta由Xi.y(Hammersley采样随机值) 和 roughness得到
    // roughness越大，theta越大，也就是H与N的夹角越大，这个与材质有关，与概率无关
    // Xi.y(Hammersley采样随机值)越大，theta越小，也就是H与N的夹角越小，可见概率高的都是夹角小的，这就是重要性采样
    // 此公式的推理比较复杂，有空再研究一下
    //
    // theta: Xi.yと粗さに依存
    // 粗さ↑→theta↑→HとNの角度↑（材質特性）
    // Xi.y↑→theta↓→角度↓（重要度サンプリングの本質）
    float theta = acos(sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y)));

    // 球面座標 → デカルト座標変換
    vec3 H;
    H.x = sin(theta) * cos(phi);
    H.y = sin(theta) * sin(phi);
    H.z = cos(theta);

    // 接空間座標 → ワールド座標変換
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = normalize(cross(N, tangent));
    mat3 TBN = mat3(tangent, bitangent, N);

    vec3 sampleVec = normalize(TBN * H);

    return sampleVec;
}

// GGX分布に基づく現在サンプルのPDF計算
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}