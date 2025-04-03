#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	int shininess;
};

uniform Material material;

void main()
{
	// 这里3个输出都没有alpha分量，所以不能开启blend
	// 3つの出力バッファにアルファ成分が存在しないため、ブレンド処理は不可
	gPosition = FragPos;

	gNormal = normalize(Normal); 

	gAlbedoSpec.rgb = texture(material.texture_diffuse1, TexCoord).rgb;
	gAlbedoSpec.a = texture(material.texture_specular1, TexCoord).r;
}