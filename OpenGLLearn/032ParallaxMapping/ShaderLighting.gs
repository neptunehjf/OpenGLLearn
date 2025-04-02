#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec3 fragPos;
	vec3 normal;         //  既可以直接用顶点数据的normal，也可以在几何着色器计算normal。之前理解错了
	vec2 texCoord;
    vec4 position;
    mat4 view;
	mat4 projection;	
    vec4 fragPosLightSpace;
    mat3 TBN;
    vec3 TangentViewPos;
	vec3 TangentFragPos;
} gs_in[];

out GS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
    vec4 fragPosLightSpace;
    mat3 TBN;
    vec3 TangentViewPos;
	vec3 TangentFragPos;
} gs_out;

uniform float magnitude;

vec3 GetNormal()
{
    vec3 a = vec3(gs_in[0].position - gs_in[1].position);
    vec3 b = vec3(gs_in[2].position - gs_in[1].position);

    return -normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    vec3 direction = magnitude * normal;
    return position + vec4(direction, 0.0);
}

void main() 
{    
    //vec3 normal = GetNormal();                 
    vec3 normal = normalize(gs_in[1].normal);

    // 如果涉及到在特定坐标空间进行计算的，要确保在对应的空间计算后，然后再进行剩下的矩阵变换（view projection之类的）
    gl_Position = gs_in[0].projection * gs_in[0].view * explode(gs_in[0].position, normal);    // 1
    gs_out.fragPos = gs_in[0].fragPos;
    gs_out.normal = gs_in[0].normal;
    gs_out.texCoord = gs_in[0].texCoord;
    gs_out.fragPosLightSpace = gs_in[0].fragPosLightSpace;
    gs_out.TBN = gs_in[0].TBN;
    gs_out.TangentViewPos = gs_in[0].TangentViewPos;
    gs_out.TangentFragPos = gs_in[0].TangentFragPos;
    EmitVertex();   

    gl_Position = gs_in[1].projection * gs_in[1].view * explode(gs_in[1].position, normal);    // 2
    gs_out.fragPos = gs_in[1].fragPos;
    gs_out.normal = gs_in[1].normal;
    gs_out.texCoord = gs_in[1].texCoord;
    gs_out.fragPosLightSpace = gs_in[1].fragPosLightSpace;
    gs_out.TBN = gs_in[1].TBN;
    gs_out.TangentViewPos = gs_in[1].TangentViewPos;
    gs_out.TangentFragPos = gs_in[1].TangentFragPos;
    EmitVertex();

    gl_Position = gs_in[2].projection * gs_in[2].view * explode(gs_in[2].position, normal);    // 3
    gs_out.fragPos = gs_in[2].fragPos;
    gs_out.normal = gs_in[2].normal;
    gs_out.texCoord = gs_in[2].texCoord;
    gs_out.fragPosLightSpace = gs_in[2].fragPosLightSpace;
    gs_out.TBN = gs_in[2].TBN;
    gs_out.TangentViewPos = gs_in[2].TangentViewPos;
    gs_out.TangentFragPos = gs_in[2].TangentFragPos;
    EmitVertex();

    EndPrimitive();    
}