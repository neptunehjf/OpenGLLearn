#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;
uniform vec3 uni_lightPos;

out vec3 fragPos;
out vec3 normal;
out vec3 lightPos;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // ע�����任��˳���Ǵ�������

  fragPos = vec3(uni_view * vec4(aPos, 1.0));
  normal = mat3(transpose(inverse(uni_view))) * aNormal; //transpose(inverse(uni_view) �˲�����GPU������ʵ�ʿ���ʱ�����ܷŵ�CPU����
  lightPos = vec3(uni_view * vec4(uni_lightPos, 1.0));
  //lightPos = mat3(uni_view) * uni_lightPos;  // ֻ���ڷ��߾�������ڼ���ǰ��mat4��w��������ص�����Ϊ����ֻ�з������ԣ�û��λ�����ԡ����������Դλ��Ҳ�ص��ʹ���
}