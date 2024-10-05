#version 330 core

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

out vec4 color;

void main()
{
	float depthValue = texture(texture_diffuse1, TexCoords).r;
	color = vec4(vec3(depthValue), 1.0);
}