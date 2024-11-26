#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // 环境cubemap
uniform float roughness; // 粗糙度

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint num);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

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
        // V是固定的，因为这里就是要求V方向上的反射颜色，并存入cubemap
        // vec3 L  = normalize(2.0 * dot(V, H) * H - V); 效果应该是一样的 
        vec3 L = normalize(reflect(-V, H)); 

        // 由于H是重要性采样得到的，算出的L集中在宏观入射角I附近，这里再次根据方差多次加权，可以提高离I近的颜色的权重
        float weight = max(dot(L, I), 0.0);
        prefilterColor += (texture(texture_cubemap1, L).rgb * weight);
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
    // 从Hammersley分布得到对应的Tangent空间的球面坐标
    // phi由Xi.x(Hammersley采样下标)得到，与概率无关
    float phi = 2 * PI * Xi.x;

    // theta由Xi.y(Hammersley采样随机值) 和 roughness得到
    // roughness越大，theta越大，也就是H与N的夹角越大，这个与材质有关，与概率无关
    // Xi.y(Hammersley采样随机值)越大，theta越小，也就是H与N的夹角越小，可见概率高的都是夹角小的，这就是重要性采样
    // 此公式的推理比较复杂，有空再研究一下
    float theta = acos(sqrt((1.0 - Xi.y) / (1.0 + (roughness * roughness - 1.0) * Xi.y)));

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