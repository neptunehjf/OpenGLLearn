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

void main()
{
    gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);
}