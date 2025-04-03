﻿#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};
uniform mat4 uni_model;

void main()
{
    gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);

    FragPos = vec3(uni_model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(uni_model))) * aNormal;
    TexCoords = aTexCoord;    
}