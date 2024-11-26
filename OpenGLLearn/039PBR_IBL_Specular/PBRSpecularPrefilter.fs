#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // ����cubemap
uniform float roughness; // �ֲڶ�

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint num);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

void main()
{
    // ��Ϊ��������ӽǷ����йأ���Ԥ�����޷�֪���ӽǷ���
    // ���Եķ����Ǽ�������V,N,I��ͬһ��ֱ����
    vec3 N = normalize(localPos);    // ����Ƭ�εĺ�۷��ߣ����õ��ھֲ�����
    vec3 I = N;                      // �����������   
    vec3 V = N;                      // ��۷�������

    vec3 prefilterColor = vec3(0.0);
    const uint samplesNum = 1024u;
    float totalWeight = 0.0;

    for (uint i = 0u; i < samplesNum; i++)
    {
        // ���ɶ�ά���������
        vec2 Xi = Hammersley(i, samplesNum);             
        // ����΢�۱���İ��������ͨ����Ҫ�Բ�������Ҫ�����ں�۱���ķ���N����
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // ���ݺ�۵ķ��䷽��V �� ΢�۵�H�����΢�۵����䷽��ķ�����L�������ڲ�������cubemap������
        // V�ǹ̶��ģ���Ϊ�������Ҫ��V�����ϵķ�����ɫ��������cubemap
        // vec3 L  = normalize(2.0 * dot(V, H) * H - V); Ч��Ӧ����һ���� 
        vec3 L = normalize(reflect(-V, H)); 

        // ����H����Ҫ�Բ����õ��ģ������L�����ں�������I�����������ٴθ��ݷ����μ�Ȩ�����������I������ɫ��Ȩ��
        float weight = max(dot(L, I), 0.0);
        prefilterColor += (texture(texture_cubemap1, L).rgb * weight);
        totalWeight += weight;
    }

    // ���Ȩ���ƽ��ֵ
    prefilterColor = prefilterColor / totalWeight;

    FragColor = vec4(prefilterColor, 1.0);
}


// Van Der Corput ���� ���ɵ���Ȼ������������������ֲ������ȣ�����ֵ�ǹ�һ����0��1֮���С��
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

// Hammersley ��Van Der Corput ����Ϊ������������һ���ά�������
vec2 Hammersley(uint i, uint num)
{
    return vec2(float(i)/float(num), RadicalInverse_VdC(i));
}

// ��Ҫ�Բ���������Hammersley��ά��������ó�һ������Ŀ��pdf����Ҫ������N���������ܵ�roughnessӰ�죩��H����
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    // ��Hammersley�ֲ��õ���Ӧ��Tangent�ռ����������
    // phi��Xi.x(Hammersley�����±�)�õ���������޹�
    float phi = 2 * PI * Xi.x;

    // theta��Xi.y(Hammersley�������ֵ) �� roughness�õ�
    // roughnessԽ��thetaԽ��Ҳ����H��N�ļн�Խ�����������йأ�������޹�
    // Xi.y(Hammersley�������ֵ)Խ��thetaԽС��Ҳ����H��N�ļн�ԽС���ɼ����ʸߵĶ��Ǽн�С�ģ��������Ҫ�Բ���
    // �˹�ʽ������Ƚϸ��ӣ��п����о�һ��
    float theta = acos(sqrt((1.0 - Xi.y) / (1.0 + (roughness * roughness - 1.0) * Xi.y)));

    // �������� --���ѿ�������
    vec3 H;
    H.x = sin(theta) * cos(phi);
    H.y = sin(theta) * sin(phi);
    H.z = cos(theta);

    // �������� --����������
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = normalize(cross(N, tangent));
    mat3 TBN = mat3(tangent, bitangent, N);

    vec3 sampleVec = normalize(TBN * H);

    return sampleVec;
}