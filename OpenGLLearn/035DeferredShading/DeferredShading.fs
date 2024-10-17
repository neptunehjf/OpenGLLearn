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
	float constant;  // 光源衰减模型的常数部分，通常为1，为了保证分母一定比分子大，不然可能出现光照反而变强的情况
	float linear;    // 一次项系数，距离较小时，一次项影响大。系数越小衰减越慢
	float quadratic; // 二次项系数，距离较大时，二次项影响大。系数越小衰减越慢
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
	// 从g-buffer缓冲获取到对应的信息
    vec3 position  = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 normal    = texture(material.texture_diffuse2, TexCoords).rgb;
    vec3 albedo    = texture(material.texture_diffuse3, TexCoords).rgb;
	float specular = texture(material.texture_diffuse3, TexCoords).a;

	// 延迟渲染，只需每个屏幕像素渲染一次就好了，可以提升很多性能
	FragColor = CalcPointLight(position, normal, albedo, specular);

	// HDR用
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
		// 环境光照ambient
	    vec4 ambient = vec4(pointLight[i].ambient * diffuseColor, 1.0);

		// 漫反射光照diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse * diffuseColor, 1.0);
	
		// 镜面光照specular
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

		// 片段离光源的距离
		float distance = length(pointLight[i].lightPos - fragPos);
		// 计算光照衰减，这里是一个点光源的衰减模型。距离较小时衰减得慢（一次项影响大）；距离较大时衰减得快（二次项影响大）；然后缓慢接近0（分母是无穷大，衰减到0）
		float lightFade = 1.0;
		if (atte_formula == 0)
			lightFade = 1.0 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);
		else if (atte_formula == 1)
			lightFade = 1.0 / (0.1 * distance);
		else if (atte_formula == 2)
			lightFade = 1.0 / (0.1 * distance * distance); // 如果不启用gamma校正，lightFade经过显示器输出会变成lightFade的2.2次方，因此算法就不对了
														   // 不启用gamma校正，则因为贴图自身有gamma校正也可以正常显示，但涉及到复杂算法就不一样了
														   // 不启用gamma校正，相当于只有贴图gamma校正，光照算法却没有gamma校正，是错误的
														   // 启用gamma校正，贴图和算法一起在最后gamma校正，是正确的
														   // 说白了，就是空间转换与算法的先后问题，之前在3D空间进行矩阵计算也遇到过。
														   // 是先把贴图转为非线性空间，在非线性空间进行光照算法计算，还是把贴图转成线性空间，在线性空间进行计算，最后转成非线性空间(gamma校正)
		// 应用光照衰减
		ambient  *= lightFade;
		diffuse  *= lightFade;
		specular *= lightFade;

		color += (ambient + diffuse + specular);
	}

	return color;
}