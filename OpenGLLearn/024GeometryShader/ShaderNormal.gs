#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT
{
	vec4 normal;
    mat4 projection;
} gs_in[];

uniform float normal_len;

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);

    return normalize(cross(a, b));
}

void main() 
{    
    vec4 normal = normalize(vec4(gs_in[1].normal));

    gl_Position = gl_in[1].gl_Position;                      // 1
    EmitVertex();   

    gl_Position = gl_in[1].gl_Position + normal * normal_len;    // 2
    EmitVertex();

    EndPrimitive();    
}