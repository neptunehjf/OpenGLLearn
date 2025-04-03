#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 ptLightSpace[6];

out vec4 FragPos;

void main() 
{   
    for (int face = 0; face < 6; face++)
    {
        // レイヤー指定（キューブマップ面インデックス）
        gl_Layer = face;
        for (int i = 0; i < 3; i++)
        {
            FragPos = gl_in[i].gl_Position;

            // 現在の面用の座標変換
            gl_Position = ptLightSpace[face] * FragPos;
            EmitVertex();               
        }
        EndPrimitive();
    }
}