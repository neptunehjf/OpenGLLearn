#version 330 core

in vec3 TexCoords;

uniform samplerCube cubemap1;

out vec4 FragColor;

void main()
{
	FragColor = texture(cubemap1, TexCoords);
}