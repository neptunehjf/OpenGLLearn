#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_view;
uniform mat4 uni_projection;
uniform mat4 uni_model;

out vec3 TexCoords;

void main()
{
    // ��պ�û�н���local->world��ת�������world�������local���꣬Ҳ����ԭ��Ϊ(0,0,0)��2x2x2��������
    // view�ռ����������Ϊԭ��ģ���˼���������Զ�����X�Ļ������൱�ڰ�����������������λ������X��
    // ���������viewת�����������w���������������������֮��Ͳ��������λ����
    // ����һ�����൱��ԭ���и������������պа������������������պеĸ�����ľ���ʼ�ն���1��ע�������1��NDC��׼���豸�����޹أ���
    // �����׶��nearӦ��ԶС��1 farӦ��Զ����1��������պе�ĳ���ֿ��ܻ�������׶֮�⣬���ü���
    // �ܽ᣺�������������һ����С����պ�(2x2x2)�ڣ�ֻ��û�����λ�Ʋ����˺��Ӻܴ�Ĵ��
    mat4 view = mat4(mat3(uni_view));
    gl_Position = uni_projection * view * vec4(aPos, 1.0);  
    TexCoords = aPos;
}