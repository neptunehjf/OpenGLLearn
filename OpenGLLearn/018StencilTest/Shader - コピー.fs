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
	float constant;  // 光源衰减模型的常数部分，通常为1，为了保证分母一定比分子大，不然可能出现光照反而变强的情况
	float linear;    // 一次项系数，距离较小时，一次项影响大。系数越小衰减越慢
	float quadratic; // 二次项系数，距离较大时，二次项影响大。系数越小衰减越慢
};

struct SpotLight
{
	vec3 lightPos;    // 聚光源位置
	vec3 direction;   // 聚光源的轴方向
	float innerCos;   // 聚光源最大角度的内圈cos
	float outerCos;   // 聚光源最大角度的外圈cos
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

	// 各分量颜色混合
	fragColor = vec4(color, 1.0);

	// DepthTest
	//float depth = LinearizeDepth(gl_FragCoord.z) / far; // 为了演示除以 far
	//fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
	
}

vec3 calcDirLight()
{
	// 环境光照ambient
	vec3 ambient = dirLight.ambient * vec3(texture(material.texture_diffuse1, texCoord));

	// 漫反射光照diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * dirLight.diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
	// 镜面光照specular
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
		// 环境光照ambient
	    vec3 ambient = pointLight[i].ambient * vec3(texture(material.texture_diffuse1, texCoord));

		// 漫反射光照diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * pointLight[i].diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
		// 镜面光照specular
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = spec * pointLight[i].specular * vec3(texture(material.texture_specular1, texCoord));

		// 片段离光源的距离
		float distance = length(pointLight[i].lightPos - fragPos);
		// 计算光照衰减，这里是一个点光源的衰减模型。距离较小时衰减得慢（一次项影响大）；距离较大时衰减得快（二次项影响大）；然后缓慢接近0（分母是无穷大，衰减到0）
		float lightFade = 1 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);
		// 应用光照衰减
		ambient  *= lightFade;
		diffuse  *= lightFade;
		specular *= lightFade;
		color += (ambient + diffuse + specular);
	}

	return color;
}

vec3 calcSpotLight()
{
	// 环境光照ambient
	vec3 ambient = spotLight.ambient * vec3(texture(material.texture_diffuse1, texCoord));

	// 聚光源
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);
	vec3 lightDir = normalize(spotLight.lightPos - fragPos); //片段到spotlight的方向
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir与聚光源的轴方向 ，注意调用normalize转成单位向量

	// 计算边缘的光照衰减
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * spotLight.diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
	// 镜面光照specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * spotLight.specular * vec3(texture(material.texture_specular1, texCoord));

	return (ambient + diffuse + specular);
}