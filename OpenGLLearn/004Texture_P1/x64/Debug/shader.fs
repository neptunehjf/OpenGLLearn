#version 330 core

in vec3 color;
in vec2 texCoord;

uniform sampler2D uni_texture0;
uniform sampler2D uni_texture1;
uniform float uni_mixValue;

out vec4 fragColor;

void main()
{
	//texCoord.x = -texCoord.x; ע��ı�����ֵ����벻������������Ϊ������ֻ����
	fragColor = mix(texture(uni_texture0, texCoord), texture(uni_texture1, vec2(texCoord.x, texCoord.y)), uni_mixValue);
}