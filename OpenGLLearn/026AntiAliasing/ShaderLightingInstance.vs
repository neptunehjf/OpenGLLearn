#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 aInstAsteriod;  // ��ɫ���������֧��vec4���ͣ���mat4����4��vec4���൱��location3 4 5 6


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
} vs_out;

void main()
{
  gl_Position = uni_projection * uni_view * aInstAsteriod * vec4(aPos, 1.0);  // ע�����任��˳���Ǵ�������

  vs_out.fragPos = vec3(aInstAsteriod * vec4(aPos, 1.0));
  vs_out.normal = mat3(transpose(inverse(aInstAsteriod))) * aNormal;
  vs_out.texCoord = aTexCoord;
}