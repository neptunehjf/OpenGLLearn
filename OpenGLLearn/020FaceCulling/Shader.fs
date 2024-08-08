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

vec4 calcDirLight(vec4 diffuseColor, vec4 specularColor);
vec4 calcPointLight(vec4 diffuseColor, vec4 specularColor);
vec4 calcSpotLight(vec4 diffuseColor, vec4 specularColor);

float near = 0.1; 
float far  = 100.0; 

void main()
{
	vec4 diffuseColor = texture(material.texture_diffuse1, texCoord);
	vec4 specularColor = texture(material.texture_specular1, texCoord);

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	color += calcDirLight(diffuseColor, specularColor);
	color += calcPointLight(diffuseColor, specularColor);
	color += calcSpotLight(diffuseColor, specularColor);

	// ��Ϊ������ӻ�ʹalpha����1�Ӷ�ʧȥ���壬����Ҫ���¼���
	color.a = diffuseColor.a;

	// ��������ɫ���
	fragColor = color;
}

vec4 calcDirLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// ��������ambient
	vec4 ambient = vec4(dirLight.ambient, 1.0) * diffuseColor;

	// ���������diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diff * vec4(dirLight.diffuse, 1.0) * diffuseColor;
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec4 specular = spec * vec4(dirLight.specular, 1.0) * specularColor;

	color = ambient + diffuse + specular;

	return color;
}

vec4 calcPointLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(uni_viewPos - fragPos);

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// ��������ambient
	    vec4 ambient = vec4(pointLight[i].ambient, 1.0) * diffuseColor;

		// ���������diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse, 1.0) * diffuseColor;
	
		// �������specular
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec4 specular = spec * vec4(pointLight[i].specular, 1.0) * specularColor;

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

vec4 calcSpotLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// ��������ambient
	vec4 ambient = vec4(spotLight.ambient, 1.0) * diffuseColor;

	// �۹�Դ
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 lightDir = normalize(spotLight.lightPos - fragPos); //Ƭ�ε�spotlight�ķ���
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir��۹�Դ���᷽�� ��ע�����normalizeת�ɵ�λ����

	// �����Ե�Ĺ���˥��
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //��clamp�Ͳ���Ҫifelse��

	// ���������diffuse
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * vec4(spotLight.diffuse, 1.0) * diffuseColor;
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * vec4(spotLight.specular, 1.0) * specularColor;

	color = ambient + diffuse + specular;

	return color;
}