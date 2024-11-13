#version 330 core
out vec4 FragColor;

in vec3 localPos;

struct Material
{
	sampler2D texture_diffuse1; // �Ⱦ�Բ��ͶӰͼ
};

uniform Material material;

// ����Cube��Ƭ��λ�������Ӧ�ĵȾ�Բ��ͶӰͼ��UV����
vec2 EquirectangularUV(vec3 v)
{
	// �ѿ������� to ��������
	float theta = asin(v.y); // ע�⣬�����theta�������������궨�弫�ǣ�������xyƽ��ļн�
	float phi = atan(v.z, v.x); // ����ǣ��˴�����Ϊ��x��ļнǣ�Ҳ���Զ���Ϊ��Z��ļнǣ�Ӱ�첻�󣬿���������ʵ���ˣ�
	
	// �������� to �Ⱦ�Բ��ͶӰͼ��UV����
	vec2 uv = vec2(phi, theta); // ��Ϊphi�Ǻ���ǣ�չ��������Ӧˮƽ����theta�Ǽ��ǵĲ��ǣ�չ��������Ӧ��ֱ����
	
	// ��׼��UV����
	// �ó���vec2(0.1591, 0.3183)��UV�����׼���� (0,1)�ķ�Χ
	// ��Ȼ����չ���ɵȾ�Բ��ͶӰͼ��y��߶Ȳ��䣬��ˮƽ����ĳ߶�����xά�Ⱥ�zά�ȵĵ��ӣ��߶ȼӱ��ˣ����Ա�׼��������ˮƽ���������Ǵ�ֱ������һ��
	const vec2 invAtan = vec2(0.1591, 0.3183);
	uv *= invAtan;
    uv += 0.5;

	return uv;
}

void main()
{
	//localPos����normalize����Ϊ�������㶼�Ǽٶ�����뾶Ϊ1��
	vec2 uv = EquirectangularUV(normalize(localPos));
	vec3 color = texture(material.texture_diffuse1, uv).rgb;
	FragColor = vec4(color, 1.0);
}