#version 330 core
layout(location = 0) in vec3 aPos;


layout (std140) uniform Matrix
{
	mat4 view;
	mat4 projection;	
};

out vec3 localPos;

void main()
{
	// 在局部空间计算即可，所以不需要model矩阵
	localPos = aPos;
	gl_Position =  projection * view * vec4(localPos, 1.0);
}