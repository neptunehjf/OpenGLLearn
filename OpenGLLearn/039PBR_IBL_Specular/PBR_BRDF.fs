#version 330 core

in vec2 TexCoords;
out vec2 FragColor; // ֻ��Ҫ�������ͨ�� R: F0�ı���  G: F0��ƫ��

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
    // Ҫ����ĸ�����V H N L

    // ����ֵֻ��NdotV�йأ�����ÿ��Ƭ�ε�N����һ��Ҳû��ϵ������ֻ��֪��N��V����Թ�ϵ��������֪��N�ľ���λ�ã�
    // ������tangent�ռ�ĺ����(y==0)���㼴��

    // ��Ϊ��tangent�ռ䣬Nȡ��λ����(0, 0, 1)
    vec3 N = vec3(0.0, 0.0, 1.0);

    // ��֪N �� NdotV,�������V
    vec3 V;
    V.z = max(NdotV, 0.0); // NdotV ���� cosTheta
    V.y = 0.0; // ��Ϊֻ�迼�Ǻ���棬�ڶ�ά�ռ���㼴��
    V.x = sqrt(1.0 - NdotV * NdotV); 

    float A = 0.0; // F0�ı����������Rͨ��
    float B = 0.0; // F0��ƫ������Gͨ�� 

    // ����ѭ��
    const uint samplesNum = 1024u;
    for (uint i = 0u; i < samplesNum; i++)
    {
        // ��Ҫ�Բ���
        // ���ɶ�ά���������
        vec2 Xi = Hammersley(i, samplesNum);             
        // ����΢�۱���İ��������ͨ����Ҫ�Բ�������Ҫ�����ں�۱���ķ���N����
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // vec3 L  = normalize(2.0 * dot(V, H) * H - V); Ч��Ӧ����һ���� 
        vec3 L = normalize(reflect(-V, H));

        // ���ˣ��Ѿ����V H N L
        // ������� NdotL NdotH VdotH�����ڼ������յ�BRDF����
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        //if (NdotL > 0.0)  // ΪʲôҪNdotL > 0.0����ע�͵�������û�����⣿
        {
            // DFG��D���ַ�Ӧ��prefilter�����ˣ����ﲻ����
            // DFG��F�����Ѿ���Ӧ�ڻ��ֵĲ�ֵ�A��B���ˣ�����ֻ�����A��B���ܷ�ӦF����
            // ����ֻ����DFG��G���ּ���
            float G = GeometrySmith(N, V, H, roughness);

            // ����Cook-Torrance BRDF����������ʽ���������Ƶ�
            // (1 / N)��Ӧ��ѭ�����ˣ����ﲻ����
            // G_Vis => (G * NdotL) / pdf / (4 * NdotL *  NdotV) =>
            // ==> (G * NdotL) / (NdotH / (4.0 * VdotH)) / (4 * NdotL *  NdotV)
            // ==> (G * NdotL) * (4.0 * VdotH) / NdotH / (4 * NdotL *  NdotV)
            // ==> (G * VdotH) / (NdotH * NdotV)��
            float G_Vis = (G * VdotH) / (NdotH * NdotV); 

            // ���ݲ��ʽ���A �� B
            // F0 * G_Vis * (1- (1 - VdotH)��5�η�) + G_Vis * (1 - VdotH)��5�η�
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

// DFG ��G��ģ����������氼͹��ƽ���µ��Գ���Ӱ
// �Ƕ�Խ����ӰԽ��
// Խ�ֲڣ���ӰԽ��
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // ע��K(IBL)��K(direct)��һ��
    float r = roughness;  // ImportanceSampleGGX���õ��Ǵֲڶȵ�ƽ����������Ȼ��1�η���Ϊʲô����ʱ���ö�֪
    float k = (r*r) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// �Գ���Ӱ��Ϊ�������֣�һ��������V��N�Ƕȹ����·�����߱�͹���Ĳ�����ס����NdotVģ��
//                    ��һ��������ⷽ��L��N�Ƕȹ����·�����߱�����ȥ�Ĳ�����ס����NdotLģ��          
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
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