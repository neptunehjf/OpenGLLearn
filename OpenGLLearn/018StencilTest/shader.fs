#version 330 core

// fragment shader的输入变量都是经过光栅化插值的
// フラグメントシェーダーの入力変数は全てラスタライザによる補間処理を経ている
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

vec3 calcDirLight();
vec3 calcPointLight();
vec3 calcSpotLight();

float near = 0.1; 
float far  = 100.0; 

//float LinearizeDepth(float depth) 
//{
//    float z = depth * 2.0 - 1.0; // [0,1] => [-1,1] （=>NDC）
//    return (2.0 * near * far) / (far + near - z * (far - near));   // 非線形 => 線形
//}

void main()
{
    vec3 color = vec3(0.0);

	color += calcDirLight();
	color += calcPointLight();
	color += calcSpotLight();

	fragColor = vec4(color, 1.0);

	// DepthTest
	//float depth = LinearizeDepth(gl_FragCoord.z) / far; // 为了演示除以 far
	                                                      // 可視化デモ用にfar値で割り算
	//fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
	
}

vec3 calcDirLight()
{
	// 环境光照ambient
	// 環境光
	vec3 ambient = dirLight.ambient * vec3(texture(material.texture_diffuse1, texCoord));

	// 漫反射光照diffuse
	// 拡散反射光 
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * dirLight.diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
	// 镜面光照specular
	// 鏡面反射光
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
		// 環境光
	    vec3 ambient = pointLight[i].ambient * vec3(texture(material.texture_diffuse1, texCoord));

		// 漫反射光照diffuse
		// 拡散反射光 
		vec3 lightDir = normalize(pointLight[i].lightPos - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * pointLight[i].diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
		// 镜面光照specular
		// 鏡面反射光
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = spec * pointLight[i].specular * vec3(texture(material.texture_specular1, texCoord));


	    //　参照　Referrence/point light attenuation.png		

		// 片段离光源的距离
	    // フラグメントからライトまでの距離
		
		float distance = length(pointLight[i].lightPos - fragPos);

		float lightFade = 1 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);

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
	// 環境光
	vec3 ambient = spotLight.ambient * vec3(texture(material.texture_diffuse1, texCoord));

	// 聚光源
	// スポットライト
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);
	vec3 lightDir = normalize(spotLight.lightPos - fragPos); //片段到spotlight的方向
								 //フラグメントからスポットライトまでの方向
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir与聚光源的轴方向 ，注意调用normalize转成单位向量
										// spotDirとスポットライト軸方向のなす角のコサイン値（normalizeを呼び単位ベクトルに変換必須）
	// 计算光照衰减
	// 光の減衰を計算する
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
    // 拡散反射光 
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * spotLight.diffuse * vec3(texture(material.texture_diffuse1, texCoord));
	
	// 镜面光照specular
    // 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * spotLight.specular * vec3(texture(material.texture_specular1, texCoord));

	return (ambient + diffuse + specular);
}