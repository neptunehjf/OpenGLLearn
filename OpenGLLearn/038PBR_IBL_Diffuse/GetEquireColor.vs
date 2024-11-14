#version 330 core
layout(location = 0) in vec3 aPos;

// �͵��Դ��Ӱһ������Ҫ���ض�λ����6���ӽ������ƣ�ֻ�������Դ���Թ�Դ�ӽǣ�������������ԭ���ӽ�	
uniform mat4 transforms;  // ��¼��ǰ���transforms�� Ҳ���� projection * view

out vec3 localPos;

void main()
{
	// �ھֲ��ռ���㼴�ɣ����Բ���Ҫmodel����
	localPos = aPos;
	gl_Position =  transforms * vec4(localPos, 1.0);
}