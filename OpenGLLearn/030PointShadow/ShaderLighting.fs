#version 330 core

in GS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;
} gs_in;

uniform vec3 uni_viewPos;
uniform samplerCube texture_cubemap1;
uniform int light_model;
uniform int atte_formula;
uniform sampler2D shadowmap;
uniform bool bShadow;
uniform bool bBias;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflection1;
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

#define POINT_LIGHT_NUM 4

uniform Material material;
uniform DirectionLight dirLight;
uniform PointLight pointLight[POINT_LIGHT_NUM];
uniform SpotLight spotLight;

out vec4 fragColor;

vec4 CalcDirLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcPointLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcSpotLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcReflectionLight(vec4 reflectionColor);
float CalcShadow(vec3 norm, vec3 lightDir);

float near = 0.1; 
float far  = 100.0; 

void main()
{
	vec4 diffuseColor = texture(material.texture_diffuse1, gs_in.texCoord);
	vec4 specularColor = texture(material.texture_specular1, gs_in.texCoord);
	vec4 reflectionColor = texture(material.texture_reflection1, gs_in.texCoord);

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	color += CalcDirLight(diffuseColor, specularColor);
	color += CalcPointLight(diffuseColor, specularColor);
	color += CalcSpotLight(diffuseColor, specularColor);
	color += CalcReflectionLight(reflectionColor);

	// ��Ϊ������ӻ�ʹalpha����1�Ӷ�ʧȥ���壬����Ҫ���¼���
	color.a = diffuseColor.a;
	// ��������ɫ���
	fragColor = color;
}

vec4 CalcDirLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// ��������ambient
	vec4 ambient = vec4(dirLight.ambient, 1.0) * diffuseColor;

	// ���������diffuse
	vec3 norm = normalize(gs_in.normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diff * vec4(dirLight.diffuse, 1.0) * diffuseColor;
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);

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

	vec4 specular = spec * vec4(dirLight.specular, 1.0) * specularColor;

	float shadow = 0.0;
	if (bShadow)
		shadow = CalcShadow(norm, lightDir);
	
	color = ambient + (1.0 - shadow) * (diffuse + specular);

	return color;
}

vec4 CalcPointLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 norm = normalize(gs_in.normal);
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// ��������ambient
	    vec4 ambient = vec4(pointLight[i].ambient, 1.0) * diffuseColor;

		// ���������diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - gs_in.fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse, 1.0) * diffuseColor;
	
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
		float distance = length(pointLight[i].lightPos - gs_in.fragPos);
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

vec4 CalcSpotLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// ��������ambient
	vec4 ambient = vec4(spotLight.ambient, 1.0) * diffuseColor;

	// �۹�Դ
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 lightDir = normalize(spotLight.lightPos - gs_in.fragPos); //Ƭ�ε�spotlight�ķ���
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir��۹�Դ���᷽�� ��ע�����normalizeת�ɵ�λ����

	// �����Ե�Ĺ���˥��
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //��clamp�Ͳ���Ҫifelse��

	// ���������diffuse
	vec3 norm = normalize(gs_in.normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * vec4(spotLight.diffuse, 1.0) * diffuseColor;
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);
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
	specular = intensity * spec * vec4(spotLight.specular, 1.0) * specularColor;

	color = ambient + diffuse + specular;

	return color;
}

vec4 CalcReflectionLight(vec4 reflectionColor)
{
	// �����reflection
	vec3 I = normalize(gs_in.fragPos - uni_viewPos);
	vec3 R = normalize(reflect(I, normalize(gs_in.normal)));
	vec4 color = reflectionColor * vec4(texture(texture_cubemap1, R).rgb, 1.0);

	return color;
}

float CalcShadow(vec3 norm, vec3 lightDir)
{
	// Shadow Mapping
	//
	// ��Ϊ��һ�����ڸ�ֵglPosition�����ģ�����û�о���glPosition������Ҫ�ֶ���һ����[-1, 1]
	// ������ͶӰû���壬��Ϊ�������[-1, 1]��wҲ��һֱ1����͸��ͶӰ��һ��ǰ�ķ�Χ��[-w, w]������Ҫ����w
	vec3 projCoords = gs_in.fragPosLightSpace.xyz / gs_in.fragPosLightSpace.w;
	// ��һ������[-1, 1] ת���� ��Ļ����[0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// ȡ���ڹ�Դ�ӽ�����Ļ����xyλ����shadowmap��Ӧ�����ֵ
	float closestDepth = texture(shadowmap, projCoords.xy).r;
	// ȡ�õ�ǰƬ���ڹ�Դ�ӽ��µ����ֵ
	float currentDepth = projCoords.z;

	float bias = 0.000; // bias���󣬿��ܻᵼ�¸�����Ӱ�ĵط�û��Ӱ�ˣ����ľ�������Ľ�û����Ӱ���������Peter-Panning����
	float shadow  = 0.0;
	if (currentDepth > 1.0) // ������׶��Χ��Ϊ����Ӱ
		shadow  = 0.0;
	else
	{
		if (bBias)
			bias = max(0.01 * (1.0 - dot(norm, lightDir)), 0.005);
		vec2 texelSize = 1.0 / textureSize(shadowmap, 0);
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				// PCF�㷨 �ữ��Ӱ���
				float pcfDepth = texture(shadowmap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
			}    
		}
		shadow /= 9.0;
	}

	return shadow;
}