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
	vec4 normal;
	mat4 projection;
} vs_out;

void main()
{
   gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // ע�����任��˳���Ǵ�������

   vs_out.normal = uni_projection * vec4(mat3(transpose(inverse(uni_view * uni_model))) * aNormal, 0.0);
   vs_out.projection = uni_projection;
}