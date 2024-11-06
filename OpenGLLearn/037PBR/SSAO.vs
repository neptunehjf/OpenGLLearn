#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

void main()
{
    // gl_Position要求是归一化的坐标，因此直接传-1到1之间的值是OK的。或者通过透视矩阵变换转成归一化坐标
    // 在离屏渲染的时候顶点着色器通常直接传归一化的值即可，但要确保顶点数据aPos确实是在-1到1之间
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;    
}