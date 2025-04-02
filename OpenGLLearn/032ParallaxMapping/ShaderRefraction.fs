#version 330 core

in VS_OUT
{
	vec3 normal;
	vec3 position;
}vs_in;

uniform samplerCube texture_cubemap1;
uniform vec3 uni_viewPos;

out vec4 FragColor;

void main()
{
	float ratio = 1.00 / 1.52;
	vec3 I = normalize(vs_in.position - uni_viewPos);
	vec3 R = normalize(refract(I, normalize(vs_in.normal), ratio));
	FragColor = vec4(texture(texture_cubemap1, R).rgb, 1.0);
}