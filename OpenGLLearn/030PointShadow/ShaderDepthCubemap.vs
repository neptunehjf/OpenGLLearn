#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_model;

void main()
{
   gl_Position = uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左
}