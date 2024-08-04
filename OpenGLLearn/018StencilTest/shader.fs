#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform vec3 uni_viewPos;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	int shininess;
};

struct DirectionLight
{
	vec3 direction; 
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 lightPos;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;  // ��Դ˥��ģ�͵ĳ������֣�ͨ��Ϊ1��Ϊ�˱�֤��ĸһ���ȷ��Ӵ󣬲�Ȼ���ܳ��ֹ��շ�����ǿ�����
	float linear;    // һ����ϵ���������Сʱ��һ����Ӱ���ϵ��ԽС˥��Խ��
	float quadratic; // ������ϵ��������ϴ�ʱ��������Ӱ���ϵ��ԽС˥��Խ��
};

struct SpotLight
{
	vec3 lightPos;    // �۹�Դλ��
	vec3 direction;   // �۹�Դ���᷽��
	float innerCos;   // �۹�Դ���Ƕȵ���Ȧcos
	float outerCos;   // �۹�Դ���Ƕȵ���Ȧcos
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define POINT_LIGHT_NUM 1

uniform Material material;
uniform DirectionLight dirLight;
uniform PointLight pointLight[POINT_LIGHT_NUM];
uniform SpotLight spotLight;

out vec4 fragColor;

vec3 calcDirLight();
vec3 calcPointLight();
vec3 calcSpotLight();

float near = 0.1; 
float far  = 100.0; 

//float LinearizeDepth(float depth) 
//{
//    float z = depth * 2.0 - 1.0; // back to NDC 
//    return (2.0 * near * far) / (far + near - z * (far - near));    
//}

void main()
{
    vec3 color = vec3(0.0);

	color += calcDirLight();
	color += calcPointLight();
	color += calcSpotLight();

	// ��������ɫ���
	fragColor = vec4(color, 1.0);

	// DepthTest
	//float depth = LinearizeDepth(gl_FragCoord.z) / far; // Ϊ����ʾ���� far
	//fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
	
}

vec3 calcDirLight()
{
	// ��������ambient
	vec3 ambient = dirLight.ambient * vec3(texture(material.texture_diffuse1, texCoord));

	// ���������diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * dirLight.diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * dirLight.specular * vec3(texture(material.texture_specular1, texCoord));

	return (ambient + diffuse + specular);
}

vec3 calcPointLight()
{
	vec3 color = vec3(0.0);
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(uni_viewPos - fragPos);

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// ��������ambient
	    vec3 ambient = pointLight[i].ambient * vec3(texture(material.texture_diffuse1, texCoord));

		// ���������diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * pointLight[i].diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
		// �������specular
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = spec * pointLight[i].specular * vec3(texture(material.texture_specular1, texCoord));

		// Ƭ�����Դ�ľ���
		float distance = length(pointLight[i].lightPos - fragPos);
		// �������˥����������һ�����Դ��˥��ģ�͡������Сʱ˥��������һ����Ӱ��󣩣�����ϴ�ʱ˥���ÿ죨������Ӱ��󣩣�Ȼ�����ӽ�0����ĸ�������˥����0��
		float lightFade = 1 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);
		// Ӧ�ù���˥��
		ambient  *= lightFade;
		diffuse  *= lightFade;
		specular *= lightFade;
		color += (ambient + diffuse + specular);
	}

	return color;
}

vec3 calcSpotLight()
{
	// ��������ambient
	vec3 ambient = spotLight.ambient * vec3(texture(material.texture_diffuse1, texCoord));

	// �۹�Դ
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);
	vec3 lightDir = normalize(spotLight.lightPos - fragPos); //Ƭ�ε�spotlight�ķ���
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir��۹�Դ���᷽�� ��ע�����normalizeת�ɵ�λ����

	// �����Ե�Ĺ���˥��
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //��clamp�Ͳ���Ҫifelse��

	// ���������diffuse
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * spotLight.diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * spotLight.specular * vec3(texture(material.texture_specular1, texCoord));

	return (ambient + diffuse + specular);
}