#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitTangent;

uniform mat4 uni_model;
uniform mat4 dirLightSpace;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

out VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 position;
	mat4 view;
	mat4 projection;
	vec4 fragPosLightSpace;
	mat3 TBN;
} vs_out;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // ע�����任��˳���Ǵ�������

  vs_out.fragPos = vec3(uni_model * vec4(aPos, 1.0));
  vs_out.normal = mat3(transpose(inverse(uni_model))) * aNormal;
  vs_out.texCoord = aTexCoord;
  vs_out.position = uni_model * vec4(aPos, 1.0);
  vs_out.view = uni_view;
  vs_out.projection = uni_projection;
  vs_out.fragPosLightSpace = dirLightSpace * vec4(vs_out.fragPos, 1.0);

  // ���߿ռ�->Local�ռ�->World�ռ� 
  // uni_model���ɷ��߾�����׼ȷ
  // �����ϣ����߱��������߿ռ䣬���������ղ���ת�����߿ռ䣬Ҳ������ȷ������ա�
  // �����Ļ���һЩ�ȽϹ̶��Ĺ��ղ������Էŵ�������ɫ����Ȼ��ת�����߿ռ�
  // ��Ϊ������ɫ���ĵ���Ƶ��ҪԶС��Ƭ����ɫ���������Ż�������
  // ������������û��������㡣������������������û���������������������һ���ԣ����û��������������������㣬���Լ�С����
  vec3 T = normalize(vec3(transpose(inverse(uni_model)) * vec4(aTangent,   0.0)));
  vec3 B = normalize(vec3(transpose(inverse(uni_model)) * vec4(aBitTangent, 0.0)));
  vec3 N = normalize(vec3(transpose(inverse(uni_model)) * vec4(aNormal,    0.0)));
  mat3 TBN = mat3(T, B, N);
  vs_out.TBN = TBN;
}