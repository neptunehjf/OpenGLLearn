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
	vec3 lightPos;  // 聚光源位置
	vec3 direction; // 聚光源的轴方向
	float innerCos;   // 聚光源最大角度的内圈cos
	float outerCos;   // 聚光源最大角度的外圈cos
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

//	float constant;  // 光源衰减模型的常数部分，通常为1，为了保证分母一定比分子大，不然可能出现光照反而变强的情况
//	float linear;    // 一次项系数，距离较小时，一次项影响大。系数越小衰减越慢
//	float quadratic; // 二次项系数，距离较大时，二次项影响大。系数越小衰减越慢
};


uniform Material material;
uniform Light light;

out vec4 fragColor;

void main()
{
	// 漫反射光照diffuse
//	vec3 norm = normalize(normal);
//	vec3 lightDir = normalize(light.lightPos - fragPos);
	//vec3 lightDir = normalize(-light.direction);
//	float diff = max(dot(norm, lightDir), 0.0);
//	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, texCoord));
	
	// 镜面光照specular
//	vec3 viewDir = normalize(uni_viewPos - fragPos);
//	vec3 reflectDir = normalize(reflect(-lightDir, norm));
//	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoord));

	// 片段离光源的距离
//	float distance = length(light.lightPos - fragPos);
	// 计算光照衰减，这里是一个点光源的衰减模型。距离较小时衰减得慢（一次项影响大）；距离较大时衰减得快（二次项影响大）；然后缓慢接近0（分母是无穷大，衰减到0）
//	float lightFade = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	// 应用光照衰减
//	ambient  *= lightFade;
//	diffuse  *= lightFade;
//	specular *= lightFade;

	// 环境光照ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));

	// 聚光源
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);
	vec3 lightDir = normalize(light.lightPos - fragPos); //片段到spotlight的方向
	float theta = max(dot(-lightDir, normalize(light.direction)), 0.0); //spotDir与聚光源的轴方向 ，注意调用normalize转成单位向量

	// 计算边缘的光照衰减
	float intensity = clamp((theta - light.outerCos) / (light.innerCos - light.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * light.diffuse * vec3(texture(material.diffuse, texCoord));
	
	// 镜面光照specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * light.specular * vec3(texture(material.specular, texCoord));

	// 各分量颜色混合
	fragColor = vec4(ambient + diffuse + specular, 1.0);
}