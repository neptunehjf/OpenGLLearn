#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout (std140) uniform Matrix
{
	mat4 view;
	mat4 projection;	
};

out vec2 TexCoords;
out mat4 Projection;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord; 
    Projection = projection;
}