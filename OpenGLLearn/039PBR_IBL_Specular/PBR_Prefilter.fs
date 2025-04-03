#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // 环境cubemap
uniform float roughness; // 粗糙度
uniform float resolution; // 环境cubemap解析度

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint num);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);

void main()
{
    // 因为镜面光与视角方向有关，而预计算无法知道视角方向。
    // 折衷的方案是假设宏观上V,N,I在同一条直线上
    vec3 N = normalize(localPos);    // 这里片段的宏观法线，正好等于局部坐标
    vec3 I = N;                      // 宏观入射向量   
    vec3 V = N;                      // 宏观反射向量

    vec3 prefilterColor = vec3(0.0);
    const uint samplesNum = 1024u;
    float totalWeight = 0.0;

    for (uint i = 0u; i < samplesNum; i++)
    {
        // 生成二维随机采样点
        vec2 Xi = Hammersley(i, samplesNum);             
        // 生成微观表面的半程向量，通过重要性采样，主要集中于宏观表面的法线N附近
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // 根据宏观的反射方向V 和 微观的H，算出微观的入射方向的反向量L，即用于采样环境cubemap的向量
        // vec3 L  = normalize(2.0 * dot(V, H) * H - V); 效果应该是一样的 
        vec3 L = normalize(reflect(-V, H)); 

        // 由于H是重要性采样得到的，算出的L集中在宏观入射角I附近，这里再次根据方差多次加权，可以提高离I近的颜色的权重
        float weight = max(dot(L, I), 0.0);

        // 算出当前采样的H的pdf
        float D = DistributionGGX(N, H, roughness); // 法线分布率
        float NdotH = max(dot(N, H), 0.0);          // 与N与H的夹角成反比
        float HdotV = max(dot(H, V), 0.0);          // 与V与H的夹角成反比
        // unreal engine4 的pdf计算公式，不是很理解是怎么来的，下面说一下自己的理解
        // pdf在D的基础上计算，NdotH(N,H夹角)额外影响了pdf，N,H夹角越小，pdf越大，但这在DistributionGGX里已经体现了，不知道为什么这里还要考虑
        // HdotV(V,H夹角)额外影响了pdf，这个可能是考虑了Fresnel方程，V与H夹角越大，反射越强
        // pdf越大和反射越强是等价的，因为重要性采样的样本集中在能让反射最强的地方
        // 另外由于V == I == N，这里实际就是 float pdf = D / 4.0 + 0.0001; 以下公式是为了适配更普适的情况
        float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;  

        float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(samplesNum) * pdf + 0.0001);

        // 根据pdf算出mipLevel，注意算出的小数会在textureLod自动向下取整
        // 这个公式暂时也不知道怎么来的。
        // 目前的理解来看，resolution越大，mipLevel越大，可能是resolution越大需要更多的采样点才能更清晰
        // SAMPLE_COUNT或者pdf越大，mipLevel越小，越清晰，这个好理解
        float mipLevel = (roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel)); 
        //float mipLevel = 0.0; 

        prefilterColor += (textureLod(texture_cubemap1, L, mipLevel).rgb * weight);
        totalWeight += weight;
    }

    // 求加权后的平均值
    prefilterColor = prefilterColor / totalWeight;

    FragColor = vec4(prefilterColor, 1.0);
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

// 算出当前采样的H的NDF
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