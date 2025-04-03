#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

uniform mat4 uni_model;

// 因为后面要用到线性空间（观察空间）的深度值，所以这里提供的FragPos和Normal也要是观察空间的，才能参与运算。
// フラグメントシェーダーへの出力（view spaceで計算）
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
	gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);

	FragPos = vec3(uni_view * uni_model * vec4(aPos, 1.0));

	Normal = mat3(transpose(inverse(uni_view * uni_model))) * aNormal;

	TexCoord = aTexCoord;
}