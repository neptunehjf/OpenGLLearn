#version 330 core

in vec2 TexCoords;
out vec2 FragColor; // 只需要输出两个通道 R: F0的比例  G: F0的偏差

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

    // 积分值只和NdotV有关，所以每个片段的N都不一样也没关系（积分只需知道N和V的相对关系，而不需知道N的绝对位置）
    // 所以在tangent空间的横截面(y==0)计算即可

    // 因为是tangent空间，N取单位向量(0, 0, 1)
    vec3 N = vec3(0.0, 0.0, 1.0);

    // 已知N 和 NdotV,可以算出V
    vec3 V;
    V.z = max(NdotV, 0.0); // NdotV 等于 cosTheta
    V.y = 0.0; // 因为只需考虑横截面，在二维空间计算即可
    V.x = sqrt(1.0 - NdotV * NdotV); 

    float A = 0.0; // F0的比例，输出到R通道
    float B = 0.0; // F0的偏差，输出到G通道 

    // 采样循环
    const uint samplesNum = 1024u;
    for (uint i = 0u; i < samplesNum; i++)
    {
        // 重要性采样
        // 生成二维随机采样点
        vec2 Xi = Hammersley(i, samplesNum);             
        // 生成微观表面的半程向量，通过重要性采样，主要集中于宏观表面的法线N附近
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // vec3 L  = normalize(2.0 * dot(V, H) * H - V); 效果应该是一样的 
        vec3 L = normalize(reflect(-V, H));

        // 至此，已经算出V H N L
        // 还需算出 NdotL NdotH VdotH，用于计算最终的BRDF积分
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        //if (NdotL > 0.0)  // 为什么要NdotL > 0.0？先注释掉看看有没有问题？
        {
            // DFG的D部分反应在prefilter部分了，这里不计算
            // DFG的F部分已经反应在积分的拆分的A和B里了，所以只需计算A和B就能反应F部分
            // 这里只计算DFG的G部分即可
            float G = GeometrySmith(N, V, H, roughness);

            // 根据Cook-Torrance BRDF的黎曼和形式，有如下推导
            // (1 / N)反应在循环外了，这里不计算
            // G_Vis => (G * NdotL) / pdf / (4 * NdotL *  NdotV) =>
            // ==> (G * NdotL) / (NdotH / (4.0 * VdotH)) / (4 * NdotL *  NdotV)
            // ==> (G * NdotL) * (4.0 * VdotH) / NdotH / (4 * NdotL *  NdotV)
            // ==> (G * VdotH) / (NdotH * NdotV)；
            float G_Vis = (G * VdotH) / (NdotH * NdotV); 

            // 根据拆分式算出A 和 B
            // F0 * G_Vis * (1- (1 - VdotH)的5次方) + G_Vis * (1 - VdotH)的5次方
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
vec2 Hammersley(uint i, uint num)
{
    return vec2(float(i)/float(num), RadicalInverse_VdC(i));
}

// DFG 的G，模拟了物体表面凹凸不平导致的自成阴影
// 角度越大，阴影越重
// 越粗糙，阴影越重
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // 注意K(IBL)和K(direct)不一样
    float r = roughness;  // ImportanceSampleGGX里用的是粗糙度的平方，这里仍然是1次方，为什么？暂时不得而知
    float k = (r*r) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 自成阴影分为两个部分，一个是视线V和N角度过大导致反射光线被凸出的部分遮住，用NdotV模拟
//                    另一个是入射光方向L与N角度过大导致反射光线被凹进去的部分遮住，用NdotL模拟          
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 重要性采样，根据Hammersley二维随机样本得出一个符合目标pdf（主要集中在N附近，并受到roughness影响）的H向量
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    // unreal engine4 用的是粗糙度的平方，效果更好
    float a = roughness * roughness;

    // 从Hammersley分布得到对应的Tangent空间的球面坐标
    // phi由Xi.x(Hammersley采样下标)得到，与概率无关
    float phi = 2 * PI * Xi.x;

    // theta由Xi.y(Hammersley采样随机值) 和 roughness得到
    // roughness越大，theta越大，也就是H与N的夹角越大，这个与材质有关，与概率无关
    // Xi.y(Hammersley采样随机值)越大，theta越小，也就是H与N的夹角越小，可见概率高的都是夹角小的，这就是重要性采样
    // 此公式的推理比较复杂，有空再研究一下
    float theta = acos(sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y)));

    // 球面坐标 --》笛卡尔坐标
    vec3 H;
    H.x = sin(theta) * cos(phi);
    H.y = sin(theta) * sin(phi);
    H.z = cos(theta);

    // 切线坐标 --》世界坐标
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = normalize(cross(N, tangent));
    mat3 TBN = mat3(tangent, bitangent, N);

    vec3 sampleVec = normalize(TBN * H);

    return sampleVec;
}