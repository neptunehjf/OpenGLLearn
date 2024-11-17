#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // ����cubemap

const float PI = 3.14159265359;

vec3 convolution(vec3 normal);

void main()
{
	// Ƭ�εķ��߷�����Ϊ��������
	vec3 normal = normalize(localPos);

	vec3 irradiance = convolution(normal);

	FragColor = vec4(irradiance, 1.0);
}

// �þ�����Ƭ�εķ����
vec3 convolution(vec3 normal)
{
	vec3 irradiance = vec3(0.0);
	
	// ��normalΪz�ṹ��һ����������ϵ
	// ��Ϊnormal��һ���ֲ��ռ�����꣬������normal���������ϵ��һ���ֲ��ռ������ϵ
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal)); // up��ʼֵ��ʲô����ν�����ﶼ�����һ����normal��ֱ��ֵ
	up = normalize(cross(normal, right));
	mat3 TBN = mat3(right, up, normal);

	// �ڷ����ķ��򣨲������򣩵İ����ڣ�����ѡȡ������ɢ����⣬Ȼ�������Ե�ƽ��ֵ�������ͣ�
	// ���� �����phi:0~2PI  ����theta:0~0.5PI
	int sampleNum = 0;
	float sampleDelta;

	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// tangent�ռ� �������� -���ѿ�������
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent�ռ� -��local�ռ�
			vec3 sample = tangentSample * TBN;

			irradiance += texture(texture_cubemap1, sample).rgb * cos(theta) * sin(theta); 
			sampleNum++;
		}
	}

	// c �� Kd �ŵ���Ӧ������ѧ����ĵط����㣬�������Ҳ����㣨ʵ��Ч��û����
	irradiance = PI * irradiance * (1.0 / (float(sampleNum))); 
	
	return irradiance;
}