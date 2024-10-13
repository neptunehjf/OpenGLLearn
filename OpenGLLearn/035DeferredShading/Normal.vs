#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_model;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

out VS_OUT
{
	vec4 normal;
} vs_out;

void main()
{
   gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // ע�����任��˳���Ǵ�������

   // 1 mat3(transpose(inverse(uni_view * uni_model))) �Ƿ��߾�������ȥ����Ϊ���ȱ����ŵ��µķ��߷������
   // 2 ������ֻ�Ƿ�����λ���޹أ�����mat3ȥ����η���w
   // 3 ��Ϊ model�����漰scale rotation translation����view������Ҫ��model�����Ľ����һ�����㣬����3D�ռ�ı任
   // 4 ��projectionֻ���ڲü���͸�ӵȣ�ʵ���Ͽ��Կ����� view * model * pos�ĺ��ڴ������ھ�����2D��Ļ�ϵ���ʾЧ��
   // 5 ���Է��߾���ֻ�ܶ�view model�ռ����3D��������projectionû��ϵ��ǿ������projection�����
   // 6 ��Ϊ����Ҫ��view�ռ�������ߣ�����transpose(inverse(uni_view * uni_model)
   // 7 ���������������ռ��㣬����model�ռ䣬����ֻҪtranspose(inverse(uni_model)
   vs_out.normal = uni_projection * vec4(mat3(transpose(inverse(uni_view * uni_model))) * aNormal, 0.0);
}