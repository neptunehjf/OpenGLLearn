#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform vec3 uni_viewPos;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	int shininess;
};

struct Light
{
	vec3 lightPos; 
	//vec3 direction; // ����ƽ�й���λ�ù�Դ������
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;  // ��Դ˥��ģ�͵ĳ������֣�ͨ��Ϊ1��Ϊ�˱�֤��ĸһ���ȷ��Ӵ󣬲�Ȼ���ܳ��ֹ��շ�����ǿ�����
	float linear;    // һ����ϵ���������Сʱ��һ����Ӱ���ϵ��ԽС˥��Խ��
	float quadratic; // ������ϵ��������ϴ�ʱ��������Ӱ���ϵ��ԽС˥��Խ��
};

uniform Material material;
uniform Light light;

out vec4 fragColor;

void main()
{
	// ��������ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	
	// ���������diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.lightPos - fragPos);
	//vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, texCoord));
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoord));

	// Ƭ�����Դ�ľ���
	float distance = length(light.lightPos - fragPos);
	// �������˥����������һ�����Դ��˥��ģ�͡������Сʱ˥��������һ����Ӱ��󣩣�����ϴ�ʱ˥���ÿ죨������Ӱ��󣩣�Ȼ�����ӽ�0����ĸ�������˥����0��
	float lightFade = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	// Ӧ�ù���˥��
	ambient  *= lightFade;
	diffuse  *= lightFade;
	specular *= lightFade;

	// ������ɫ��������ɫ��ϣ�Ӫ�����Ч��
	fragColor = vec4(ambient + diffuse + specular, 1.0);
}