#version 330 core

layout (triangles) in;
layout (triangles_strip, max_vertices = 18) out;

uniform mat4 ptLightSpace[6];

out vec4 FragPos;

void main() 
{   
    for (int face = 0; face < 6; i++)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; i++)
        {
            FragPos = gl_in[i].gl_Position; // Ϊ����fragment shader�������ֵ��Ҳ������opengl�Զ�������ȣ������Լ�����Ƚ�ֱ��
            gl_Position = ptLightSpace[face] * gl_in[i].gl_Position; //��ü�����׶���Ƭ��
            EmitVertex();               
        }
        EndPrimitive();
    }
}