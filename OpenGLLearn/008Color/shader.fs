#version 330 core

uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;
//uniform bool uni_isLight;  // ��Ȼ��shader.fs��if else���Ҳ�ֱܷ���Ⱦ��Դ���������ɫ�����ǵ���дһ��shader�����ĺô�

out vec4 fragColor;

void main()
{
	fragColor = vec4(uni_objectColor * uni_lightColor, 1.0);
}