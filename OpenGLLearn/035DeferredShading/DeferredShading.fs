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
//                    // 減衰モデルの定数項（通常1.0 分母が分子を超えるように）
	float linear;    // 一次项系数，距离较小时，一次项影响大。系数越小衰减越慢
//					　//　一次減衰係数 近距離で影響大　 ※係数小=減衰遅い
	float quadratic; // 二次项系数，距离较大时，二次项影响大。系数越小衰减越慢
//　　　　　　　　　　　//　二次減衰係数 遠距離で影響大　 ※係数小=減衰遅い
	float radius;    // for light vomlume optimization
};

uniform Material material;
uniform vec3 uni_viewPos;

#define POINT_LIGHT_NUM 96
uniform PointLight pointLight[POINT_LIGHT_NUM];

uniform int light_model;
uniform int atte_formula;

uniform bool bLightVolume;

uniform int iGPUPressure;

vec4 CalcPointLight(vec3 fragPos, vec3 norm, vec3 diffuseColor, float specularColor);

void main()
{   
	// Gバッファから各情報を取得
    vec3 position  = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 normal    = texture(material.texture_diffuse2, TexCoords).rgb;
    vec3 albedo    = texture(material.texture_diffuse3, TexCoords).rgb;
	float specular = texture(material.texture_diffuse3, TexCoords).a;

	// 延迟渲染，只需每个屏幕像素渲染一次就好了，可以提升很多性能
	// ディファードレンダリングの核心：ピクセルごとに1回のみ計算 → パフォーマンス最適化

	// 用重复渲染的方式来给GPU压力，用于测试延迟渲染的优化效果。当然也可以用传入大量光源的方式，但是比较麻烦。
	// ※意図的なGPU負荷テスト用ループ（実際の開発では光源数制御が一般的）
	for (int i = 0; i < iGPUPressure; i++)
	{
		FragColor = vec4(1.0);
		FragColor = CalcPointLight(position, normal, albedo, specular);
	}

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
		// 光体积以外的片段不计算光照
		// ライトボリューム外のフラグメントはライティング計算をスキップ（パフォーマンス最適化）
		float distance = length(pointLight[i].lightPos - fragPos);
        if(bLightVolume && distance > pointLight[i].radius)
			continue;

		// ambient
	    vec4 ambient = vec4(pointLight[i].ambient * diffuseColor, 1.0);

		// diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse * diffuseColor, 1.0);
	
		// specular
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

		float lightFade = 1.0;
		if (atte_formula == 0)
			lightFade = 1.0 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);
		else if (atte_formula == 1)
			lightFade = 1.0 / (0.1 * distance);
		else if (atte_formula == 2)
			lightFade = 1.0 / (0.1 * distance * distance);

		ambient  *= lightFade;
		diffuse  *= lightFade;
		specular *= lightFade;

		color += (ambient + diffuse + specular);
	}

	return color;
}