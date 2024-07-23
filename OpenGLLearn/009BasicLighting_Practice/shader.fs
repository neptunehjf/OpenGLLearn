#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec3 lightPos;
in vec4 fragColor;

out vec4 color;

void main()
{
	color = fragColor;
}