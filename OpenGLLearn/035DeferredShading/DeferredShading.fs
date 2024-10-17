#version 330 core

in vec2 TexCoords;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


struct Material
{
	sampler2D texture_diffuse1; // G-buffer Position vec3
    sampler2D texture_diffuse2; // G-buffer Normal vec3
    sampler2D texture_diffuse3; // G-buffer Albedo and Specular vec4
	int shininess;
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

uniform Material material;
uniform vec3 uni_viewPos;

#define POINT_LIGHT_NUM 96
uniform PointLight pointLight[POINT_LIGHT_NUM];

uniform int light_model;
uniform int atte_formula;

vec4 CalcPointLight(vec3 fragPos, vec3 norm, vec3 diffuseColor, float specularColor);

void main()
{   
	// ��g-buffer�����ȡ����Ӧ����Ϣ
    vec3 position  = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 normal    = texture(material.texture_diffuse2, TexCoords).rgb;
    vec3 albedo    = texture(material.texture_diffuse3, TexCoords).rgb;
	float specular = texture(material.texture_diffuse3, TexCoords).a;

	// �ӳ���Ⱦ��ֻ��ÿ����Ļ������Ⱦһ�ξͺ��ˣ����������ܶ�����
	FragColor = CalcPointLight(position, normal, albedo, specular);

	// HDR��
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
	    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec4 CalcPointLight(vec3 fragPos, vec3 norm, vec3 diffuseColor, float specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 viewDir = normalize(uni_viewPos - fragPos);

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// ��������ambient
	    vec4 ambient = vec4(pointLight[i].ambient * diffuseColor, 1.0);

		// ���������diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse * diffuseColor, 1.0);
	
		// �������specular
		float spec = 0.0f;
		// Phong
		if (light_model == 0)
		{
			vec3 reflectDir = normalize(reflect(-lightDir, norm));
			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		}
		// Blinn-Phong
		else if (light_model == 1)
		{
			vec3 halfwayDir = normalize(lightDir + viewDir);
			spec = pow(max(dot(halfwayDir, norm), 0.0), material.shininess);
		}
		vec4 specular = spec * vec4(pointLight[i].specular, 1.0) * specularColor;

		// Ƭ�����Դ�ľ���
		float distance = length(pointLight[i].lightPos - fragPos);
		// �������˥����������һ�����Դ��˥��ģ�͡������Сʱ˥��������һ����Ӱ��󣩣�����ϴ�ʱ˥���ÿ죨������Ӱ��󣩣�Ȼ�����ӽ�0����ĸ�������˥����0��
		float lightFade = 1.0;
		if (atte_formula == 0)
			lightFade = 1.0 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);
		else if (atte_formula == 1)
			lightFade = 1.0 / (0.1 * distance);
		else if (atte_formula == 2)
			lightFade = 1.0 / (0.1 * distance * distance); // ���������gammaУ����lightFade������ʾ���������lightFade��2.2�η�������㷨�Ͳ�����
														   // ������gammaУ��������Ϊ��ͼ������gammaУ��Ҳ����������ʾ�����漰�������㷨�Ͳ�һ����
														   // ������gammaУ�����൱��ֻ����ͼgammaУ���������㷨ȴû��gammaУ�����Ǵ����
														   // ����gammaУ������ͼ���㷨һ�������gammaУ��������ȷ��
														   // ˵���ˣ����ǿռ�ת�����㷨���Ⱥ����⣬֮ǰ��3D�ռ���о������Ҳ��������
														   // ���Ȱ���ͼתΪ�����Կռ䣬�ڷ����Կռ���й����㷨���㣬���ǰ���ͼת�����Կռ䣬�����Կռ���м��㣬���ת�ɷ����Կռ�(gammaУ��)
		// Ӧ�ù���˥��
		ambient  *= lightFade;
		diffuse  *= lightFade;
		specular *= lightFade;

		color += (ambient + diffuse + specular);
	}

	return color;
}