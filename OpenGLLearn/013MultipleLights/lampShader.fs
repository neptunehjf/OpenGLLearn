

#version 330 core

uniform vec3 uni_lightColor;

out vec4 fragColor;


void main()
{
	fragColor = vec4(uni_lightColor, 1.0);
}