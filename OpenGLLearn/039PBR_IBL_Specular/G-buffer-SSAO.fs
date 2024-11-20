#version 330 core

layout(location = 0) out vec4 gPositionDepth;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform float near; // ͶӰ����Ľ�ƽ��
uniform float far; // ͶӰ�����Զƽ��

// ת�������Կռ䣨�۲�ռ䣩
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // �ص�NDC
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
	gPositionDepth.rgb = FragPos;
	gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);

	gNormal = normalize(Normal); 

	// �������óɰ�ɫ�����Ը����׿���SSAOЧ��
	gAlbedo = vec3(0.95);
}