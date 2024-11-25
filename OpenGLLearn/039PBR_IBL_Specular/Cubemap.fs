#version 330 core

in vec3 TexCoords;

uniform samplerCube texture_cubemap1;
uniform int iMipLevel;

out vec4 FragColor;

void main()
{
	vec3 color = textureLod(texture_cubemap1, TexCoords, iMipLevel).rgb;

	// HDR tonemap
	// �ŵ�postprocess shader�ﴦ��

	// Gamma Correct ��Ϊ�Ѿ���CPU��������glEnable(GL_FRAMEBUFFER_SRGB); ������벻��Ҫ
    // color = pow(color, vec3(1.0/2.2)); 

	FragColor = vec4(color, 1.0);
}