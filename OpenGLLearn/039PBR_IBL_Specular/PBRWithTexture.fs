#version 330 core

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform bool bIBL;
uniform int iFrenselMode;
uniform bool bDirectLight;

// IBL
uniform samplerCube irradianceMap; // 辐照度cubemap
uniform samplerCube prefilterMap; // prefilter cubemap
uniform sampler2D brdfMap; // BRDF图

struct Material
{  
	sampler2D texture_diffuse1; // albedoMap
	sampler2D texture_diffuse2; // metallicMap
	sampler2D texture_diffuse3; // roughnessMap
	sampler2D texture_diffuse4; // aoMap
    sampler2D texture_diffuse5; // normalMap
};
uniform Material material;

vec3  albedo    = pow(texture(material.texture_diffuse1, TexCoords).rgb, vec3(2.2)); // 材质转成线性空间
float metallic  = texture(material.texture_diffuse2, TexCoords).r;
float roughness = texture(material.texture_diffuse3, TexCoords).r;
float ao        = texture(material.texture_diffuse4, TexCoords).r;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 GetNormalFromMap();

void main()
{
    vec3 N = GetNormalFromMap();
    //vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    vec3 L = reflect(-V, N);  

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F;
    if (iFrenselMode == 0)
        F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    else if (iFrenselMode == 1)
        F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness); 

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, L,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF  = texture(brdfMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient;
    if (bIBL)
        ambient = (kD * diffuse + specular) * ao;  // specular内部计算已经考虑Ks了，所以这里不用再乘
    else
        ambient = vec3(0.03) * albedo * ao;
    
    vec3 color;
    if (bDirectLight)
        color = ambient + Lo;
    else
        color = ambient;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    FragColor = vec4(color , 1.0);
}

// N 决定了当前位置的片段的微表面与H一致的概率，概率越高的片段，反射越强，显然N与H越重合，概率越高
// 这里的概率显然不是统计学采样算出的，而是一个大致估算。因为N和H全都是宏观的向量，不涉及微表面
// roughness 决定了整体微表面的方向的随机程度，roughness越大，反射与无反射区域的过度越平滑(让概率大的地方概率减小，反之亦然)
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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

// 简化版的取法线函数，具体原理等有空了解一下
vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(material.texture_diffuse5, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = normalize(cross(N, tangent));
    //mat3 TBN = mat3(tangent, bitangent, N);
    
    return normalize(TBN * tangentNormal);
}