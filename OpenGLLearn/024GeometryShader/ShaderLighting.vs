#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_model;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

out VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 position;
	mat4 view;
	mat4 projection;	
} vs_out;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

  vs_out.fragPos = vec3(uni_model * vec4(aPos, 1.0));
  vs_out.normal = mat3(transpose(inverse(uni_model))) * aNormal;
  vs_out.texCoord = aTexCoord;
  vs_out.position = uni_model * vec4(aPos, 1.0);
  vs_out.view = uni_view;
  vs_out.projection = uni_projection;
}