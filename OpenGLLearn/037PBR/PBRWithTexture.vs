﻿#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_model;
layout (std140) uniform Matrix
{
	mat4 view;
	mat4 projection;	
};

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
    TexCoords = aTexCoord;
    WorldPos = vec3(uni_model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(uni_model))) * aNormal;

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}