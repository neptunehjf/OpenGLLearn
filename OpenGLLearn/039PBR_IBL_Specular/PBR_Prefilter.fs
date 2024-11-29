#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // ����cubemap
uniform float roughness; // �ֲڶ�
uniform float resolution; // ����cubemap������

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint num);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);

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
        // vec3 L  = normalize(2.0 * dot(V, H) * H - V); Ч��Ӧ����һ���� 
        vec3 L = normalize(reflect(-V, H)); 

        // ����H����Ҫ�Բ����õ��ģ������L�����ں�������I�����������ٴθ��ݷ����μ�Ȩ�����������I������ɫ��Ȩ��
        float weight = max(dot(L, I), 0.0);

        // �����ǰ������H��pdf
        float D = DistributionGGX(N, H, roughness); // ���߷ֲ���
        float NdotH = max(dot(N, H), 0.0);          // ��N��H�ļнǳɷ���
        float HdotV = max(dot(H, V), 0.0);          // ��V��H�ļнǳɷ���
        // unreal engine4 ��pdf���㹫ʽ�����Ǻ��������ô���ģ�����˵һ���Լ������
        // pdf��D�Ļ����ϼ��㣬NdotH(N,H�н�)����Ӱ����pdf��N,H�н�ԽС��pdfԽ�󣬵�����DistributionGGX���Ѿ������ˣ���֪��Ϊʲô���ﻹҪ����
        // HdotV(V,H�н�)����Ӱ����pdf����������ǿ�����Fresnel���̣�V��H�н�Խ�󣬷���Խǿ
        // pdfԽ��ͷ���Խǿ�ǵȼ۵ģ���Ϊ��Ҫ�Բ������������������÷�����ǿ�ĵط�
        // ��������V == I == N������ʵ�ʾ��� float pdf = D / 4.0 + 0.0001; ���¹�ʽ��Ϊ����������ʵ����
        float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;  

        float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(samplesNum) * pdf + 0.0001);

        // ����pdf���mipLevel��ע�������С������textureLod�Զ�����ȡ��
        // �����ʽ��ʱҲ��֪����ô���ġ�
        // Ŀǰ�����������resolutionԽ��mipLevelԽ�󣬿�����resolutionԽ����Ҫ����Ĳ�������ܸ�����
        // SAMPLE_COUNT����pdfԽ��mipLevelԽС��Խ��������������
        float mipLevel = (roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel)); 
        //float mipLevel = 0.0; 

        prefilterColor += (textureLod(texture_cubemap1, L, mipLevel).rgb * weight);
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
    // unreal engine4 �õ��Ǵֲڶȵ�ƽ����Ч������
    float a = roughness * roughness;

    // ��Hammersley�ֲ��õ���Ӧ��Tangent�ռ����������
    // phi��Xi.x(Hammersley�����±�)�õ���������޹�
    float phi = 2 * PI * Xi.x;

    // theta��Xi.y(Hammersley�������ֵ) �� roughness�õ�
    // roughnessԽ��thetaԽ��Ҳ����H��N�ļн�Խ�����������йأ�������޹�
    // Xi.y(Hammersley�������ֵ)Խ��thetaԽС��Ҳ����H��N�ļн�ԽС���ɼ����ʸߵĶ��Ǽн�С�ģ��������Ҫ�Բ���
    // �˹�ʽ������Ƚϸ��ӣ��п����о�һ��
    float theta = acos(sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y)));

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

// �����ǰ������H��NDF
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