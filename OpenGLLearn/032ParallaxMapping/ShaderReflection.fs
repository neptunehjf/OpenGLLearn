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
	// 反射光的计算
	vec3 I = normalize(vs_in.position - uni_viewPos);
	vec3 R = normalize(reflect(I, normalize(vs_in.normal)));
	FragColor = vec4(texture(texture_cubemap1, R).rgb, 1.0);
}