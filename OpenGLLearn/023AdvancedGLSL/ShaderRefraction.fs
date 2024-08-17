#version 330 core

in vec3 normal;
in vec3 position;

uniform samplerCube texture_cubemap1;
uniform vec3 uni_viewPos;

out vec4 FragColor;

void main()
{
	float ratio = 1.00 / 1.52;
	vec3 I = normalize(position - uni_viewPos);
	vec3 R = normalize(refract(I, normalize(normal), ratio));
	FragColor = vec4(texture(texture_cubemap1, R).rgb, 1.0);
}