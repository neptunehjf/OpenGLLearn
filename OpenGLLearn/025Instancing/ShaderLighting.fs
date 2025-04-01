#version 330 core

in GS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} vs_in;

uniform vec3 uni_viewPos;
uniform samplerCube texture_cubemap1;

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
	float constant;  // 光源衰减模型的常数部分，通常为1，为了保证分母一定比分子大，不然可能出现光照反而变强的情况
//                    // 減衰モデルの定数項（通常1.0 分母が分子を超えるように）
	float linear;    // 一次项系数，距离较小时，一次项影响大。系数越小衰减越慢
//					　//　一次減衰係数 近距離で影響大　 ※係数小=減衰遅い
	float quadratic; // 二次项系数，距离较大时，二次项影响大。系数越小衰减越慢
//　　　　　　　　　　　//　二次減衰係数 遠距離で影響大　 ※係数小=減衰遅い
};

struct SpotLight
{

	vec3 lightPos;  // 聚光源位置
					// スポットライトの位置
	vec3 direction; // 聚光源的轴方向
					// スポットライトの軸方向
	float innerCos;   // 聚光源最大角度的内圈cosine
					  // スポットライト内側cosine値
	float outerCos;   // 聚光源最大角度的外圈cosine
				      // スポットライト外側cosine値
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
vec4 calcReflectionLight(vec4 reflectionColor);

float near = 0.1; 
float far  = 100.0; 

void main()
{
	vec4 diffuseColor = texture(material.texture_diffuse1, vs_in.texCoord);
	vec4 specularColor = texture(material.texture_specular1, vs_in.texCoord);
	vec4 reflectionColor = texture(material.texture_reflection1, vs_in.texCoord);

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	color += calcDirLight(diffuseColor, specularColor);
	color += calcPointLight(diffuseColor, specularColor);
	color += calcSpotLight(diffuseColor, specularColor);
	color += calcReflectionLight(reflectionColor);

	// 因为向量相加会使alpha超过1从而失去意义，所以要重新赋值
	// ベクトル加算によりアルファ値が1.0を超過すると意味がないため、再代入を実施
	color.a = diffuseColor.a;

	fragColor = color;
}

vec4 calcDirLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// 环境光照ambient
	// 環境光
	vec4 ambient = vec4(dirLight.ambient, 1.0) * diffuseColor;

	// 漫反射光照diffuse
	// 拡散反射光 
	vec3 norm = normalize(vs_in.normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diff * vec4(dirLight.diffuse, 1.0) * diffuseColor;
	
	// 镜面光照specular
	// 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - vs_in.fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec4 specular = spec * vec4(dirLight.specular, 1.0) * specularColor;

	color = ambient + diffuse + specular;

	return color;
}

vec4 calcPointLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 norm = normalize(vs_in.normal);
	vec3 viewDir = normalize(uni_viewPos - vs_in.fragPos);

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// 环境光照ambient
		// 環境光
	    vec4 ambient = vec4(pointLight[i].ambient, 1.0) * diffuseColor;

		// 漫反射光照diffuse
		// 拡散反射光 
		vec3 lightDir = normalize(pointLight[i].lightPos - vs_in.fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse, 1.0) * diffuseColor;
	
		// 镜面光照specular
		// 鏡面反射光
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec4 specular = spec * vec4(pointLight[i].specular, 1.0) * specularColor;


	    //　参照　Referrence/point light attenuation.png		

		// 片段离光源的距离
	    // フラグメントからライトまでの距離
		float distance = length(pointLight[i].lightPos - vs_in.fragPos);

		float lightFade = 1 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);

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

	// 环境光照ambient
	// 環境光
	vec4 ambient = vec4(spotLight.ambient, 1.0) * diffuseColor;

	// 聚光源
	// スポットライト
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 lightDir = normalize(spotLight.lightPos - vs_in.fragPos); //片段到spotlight的方向
								                             //フラグメントからスポットライトまでの方向
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir与聚光源的轴方向 ，注意调用normalize转成单位向量
										                                    // spotDirとスポットライト軸方向のなす角のコサイン値（normalizeを呼び単位ベクトルに変換必須）
	// 计算光照衰减
	// 光の減衰を計算する
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
    // 拡散反射光 
	vec3 norm = normalize(vs_in.normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * vec4(spotLight.diffuse, 1.0) * diffuseColor;
	
	// 镜面光照specular
    // 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - vs_in.fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * vec4(spotLight.specular, 1.0) * specularColor;

	color = ambient + diffuse + specular;

	return color;
}

vec4 calcReflectionLight(vec4 reflectionColor)
{
	// 反射光reflection
	vec3 I = normalize(vs_in.fragPos - uni_viewPos);
	vec3 R = normalize(reflect(I, normalize(vs_in.normal)));
	vec4 color = reflectionColor * vec4(texture(texture_cubemap1, R).rgb, 1.0);

	return color;
}