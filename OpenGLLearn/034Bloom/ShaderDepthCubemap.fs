#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
	float distance = length(FragPos.xyz - lightPos);
	gl_FragDepth = distance / farPlane; // map to [0, 1]
}

