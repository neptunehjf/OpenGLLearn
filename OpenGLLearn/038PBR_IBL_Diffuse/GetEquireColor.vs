#version 330 core
layout(location = 0) in vec3 aPos;


layout (std140) uniform Matrix
{
	mat4 view;
	mat4 projection;	
};

out vec3 localPos;

void main()
{
	// �ھֲ��ռ���㼴�ɣ����Բ���Ҫmodel����
	localPos = aPos;
	gl_Position =  projection * view * vec4(localPos, 1.0);
}