#version 330 core

in vec2 TexCoords;
out vec2 FragColor;
const float PI = 3.14159265359;

vec2 IntergrateBRDF(float NdotV, float roughness);
float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint num);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

void main()
{
	FragColor = IntergrateBRDF(TexCoords.x, TexCoords.y);
}

vec2 IntergrateBRDF(float NdotV, float roughness)
{
    // 要求出四个向量V H N L
    // 必要なベクトル：V, H, N, L

    // 积分值只和NdotV有关，所以每个片段的N都不一样也没关系（积分只需知道N和V的相对关系，而不需知道N的绝对位置）
    // 所以在tangent空间的横截面(y==0)计算即可
    //
    // 積分値はNdotVのみに依存するため、各フラグメントの法線Nが異なっても問題ありません（積分計算にはNとVの相対関係のみが必要で、Nの絶対座標は不要です）。
    // 接線空間（y==0）の断面で計算できる

    // 因为是tangent空间，N取单位向量(0, 0, 1)
    // 接空間ではNを(0,0,1)に固定
    vec3 N = vec3(0.0, 0.0, 1.0);

    // 法線NとNdotVからVを算出
    vec3 V;
    V.z = max(NdotV, 0.0);  // NdotV = cosθ
    V.y = 0.0;              // 断面計算のためY軸成分をゼロに設定（2次元平面化）
    V.x = sqrt(1.0 - NdotV * NdotV); 

    float A = 0.0; // フレネル反射率F0の係数（赤チャネル出力）
    float B = 0.0; // フレネルオフセット項（緑チャネル出力）

    // 采样循环
    // サンプリングループ
    const uint samplesNum = 1024u;
    for (uint i = 0u; i < samplesNum; i++)
    {
        // 生成二维随机采样点
        // Hammersley列でサンプリング点生成
        vec2 Xi = Hammersley(i, samplesNum);             
        // 生成微观表面的半程向量，通过重要性采样，主要集中于宏观表面的法线N附近
        // GGX重要度サンプリングによる微小面法線Hを取得
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);

        vec3 L = normalize(reflect(-V, H));

        // vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        // 至此，已经算出V H N L
        // 还需算出 NdotL NdotH VdotH，用于计算最终的BRDF积分
        //
        // BRDF計算用パラメータ
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0)  // NdotL > 0.0の条件チェック（裏面からの光を除外）
        {
            // DFG的D部分反应在prefilter部分了，这里不计算
            // DFG的F部分已经反应在积分的拆分的A和B里了，所以只需计算A和B就能反应F部分
            // 这里只计算DFG的G部分即可
            //
            // D項は事前フィルタリング済みのため計算不要
            // F項はA/Bの分離積分に包含されている
            // 幾何減衰項Gのみ計算
            float G = GeometrySmith(N, V, H, roughness);

            // 根据Cook-Torrance BRDF的黎曼和形式，有如下推导
            // (1 / N)反应在循环外了，这里不计算
            //
            // Cook-Torrance BRDFのリーマン和 導出プロセス：
            // (1 / N) はループの外側に反映され、ここでは計算されません
            //
            // G_Vis => (G * NdotL) / pdf / (4 * NdotL *  NdotV) =>
            // ==> (G * NdotL) / (NdotH / (4.0 * VdotH)) / (4 * NdotL *  NdotV)
            // ==> (G * NdotL) * (4.0 * VdotH) / NdotH / (4 * NdotL *  NdotV)
            // ==> (G * VdotH) / (NdotH * NdotV)；
            float G_Vis = (G * VdotH) / (NdotH * NdotV); 

            // 根据拆分式算出A 和 B
            // F0 * G_Vis * (1- (1 - VdotH)的5次方) + G_Vis * (1 - VdotH)的5次方
            //
            // 分離積分式に基づくA/B計算
            // Schlick Fresnel近似の分離表現
            float Fc = pow(1 - VdotH, 5.0);
            A += G_Vis * (1 - Fc);
            B += G_Vis * Fc;
        }
    }
    
    // 1 / N
    A /= float(samplesNum);
    B /= float(samplesNum);
    
    return vec2(A, B);
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

// DFG 的G，模拟了物体表面凹凸不平导致的自成阴影
// 角度越大，阴影越重
// 越粗糙，阴影越重
// DFGのG項：表面の凹凸によるセルフシャドウイングをモデル化
// 角度が大きいほど、粗さが高いほどシャドウが強くなる
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // 注意K(IBL)和K(direct)不一样
    // IBL用と直接光用では係数Kが異なる
    float r = roughness; 
    float k = (r*r) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 自成阴影分为两个部分，一个是视线V和N角度过大导致反射光线被凸出的部分遮住，用NdotV模拟
//                    另一个是入射光方向L与N角度过大导致反射光线被凹进去的部分遮住，用NdotL模拟  
// セルフシャドウイングの二重効果を計算：
// 1. 視線方向の凹凸陰影（NdotVで評価）
// 2. 入射光方向の凹凸遮蔽（NdotLで評価）
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
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