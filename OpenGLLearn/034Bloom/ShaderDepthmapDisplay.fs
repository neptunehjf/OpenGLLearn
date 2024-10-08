#version 330 core

in vec2 TexCoords;

struct Material
{
	sampler2D texture_diffuse1;
};

uniform Material material;

out vec4 color;

void main()
{
	float depthValue = texture(material.texture_diffuse1, TexCoords).r;
	color = vec4(vec3(depthValue), 1.0);
}