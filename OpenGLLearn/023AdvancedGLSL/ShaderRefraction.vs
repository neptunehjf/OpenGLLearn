#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord; // 忽略就好，只是为了复用代码

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;

out VS_OUT
{
	vec3 normal;
	vec3 position;
}vs_out;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

  vs_out.normal = mat3(transpose(inverse(uni_model))) * aNormal;
  vs_out.position = vec3(uni_model * vec4(aPos, 1.0));;
}