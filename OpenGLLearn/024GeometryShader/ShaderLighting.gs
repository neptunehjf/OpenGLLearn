#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec3 fragPos;
	vec3 normal;    // ������ɫ������ֱ���ö����Դ��ķ��ߣ���Ϊ������߲�һ���Ǵ�ֱ�ڵ�ǰ�����εģ�����һ��1��������Ķ��㣬��Ӧ3�����ߣ�
	vec2 texCoord;
} gs_in[];

out GS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} gs_out;

uniform float magnitude;

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);

    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    vec3 direction = magnitude * normal;
    return position + vec4(direction, 0.0);
}

void main() 
{    
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);    // 1
    gs_out.fragPos = gs_in[0].fragPos;
    gs_out.normal = gs_in[0].normal;
    gs_out.texCoord = gs_in[0].texCoord;
    EmitVertex();   

    gl_Position = explode(gl_in[1].gl_Position, normal);    // 2
    gs_out.fragPos = gs_in[1].fragPos;
    gs_out.normal = gs_in[1].normal;
    gs_out.texCoord = gs_in[1].texCoord;
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, normal);    // 3
    gs_out.fragPos = gs_in[2].fragPos;
    gs_out.normal = gs_in[2].normal;
    gs_out.texCoord = gs_in[2].texCoord;
    EmitVertex();

    EndPrimitive();    
}