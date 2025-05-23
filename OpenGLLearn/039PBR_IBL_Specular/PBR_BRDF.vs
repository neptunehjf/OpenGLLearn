﻿#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; // dummy
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;    
}