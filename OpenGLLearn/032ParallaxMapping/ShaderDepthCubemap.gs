#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 ptLightSpace[6];

out vec4 FragPos;

void main() 
{   
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; i++)
        {
            FragPos = gl_in[i].gl_Position; // 为了在fragment shader计算深度值，也可以让opengl自动计算深度，但是自己计算比较直观
            gl_Position = ptLightSpace[face] * FragPos; //会裁剪掉视锥外的片段
            EmitVertex();               
        }
        EndPrimitive();
    }
}