#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

void main()
{
    //gl_PointSize = 20.0;
    // gl_PositionҪ���ǹ�һ�������꣬���ֱ�Ӵ�-1��1֮���ֵ��OK�ġ�����ͨ��͸�Ӿ���任ת�ɹ�һ������
    // gl_Position����Ҏ�����ˤ�Ҫ�󤹤뤿�ᡢ-1����1�ι���΂���ֱ�Ӷɤ��Ƥ↖�}����ޤ��󡣤��뤤��OpenGL��͸ҕ�����Q��perspective division����ͨ������Ҏ�����ˤˉ�Q���뤳�Ȥ���ܤǤ���
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;    
}