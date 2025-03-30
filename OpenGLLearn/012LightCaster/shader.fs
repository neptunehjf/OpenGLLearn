#version 330 core

// fragment shader的输入变量都是经过光栅化插值的
// フラグメントシェーダーの入力変数は全てラスタライザによる補間処理を経ている
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
	// 聚光源
	// スポットライト
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

// 点光源
// ポイントライト
//	float constant;  // 光源衰减模型的常数部分，通常为1，为了保证分母一定比分子大，不然可能出现光照反而变强的情况
//                    // 減衰モデルの定数項（通常1.0 分母が分子を超えるように）
//	float linear;    // 一次项系数，距离较小时，一次项影响大。系数越小衰减越慢
//					　//　一次減衰係数 近距離で影響大　 ※係数小=減衰遅い
//	float quadratic; // 二次项系数，距离较大时，二次项影响大。系数越小衰减越慢
//　　　　　　　　　　　//　二次減衰係数 遠距離で影響大　 ※係数小=減衰遅い
};


uniform Material material;
uniform Light light;

out vec4 fragColor;

void main()
{
	// 点光源
	// ポイントライト

	// 漫反射光照diffuse
	// 拡散反射光 
//	vec3 norm = normalize(normal);
//	vec3 lightDir = normalize(light.lightPos - fragPos);
	//vec3 lightDir = normalize(-light.direction);
//	float diff = max(dot(norm, lightDir), 0.0);
//	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, texCoord));
	
	// 镜面光照specular
	// 鏡面反射光
//	vec3 viewDir = normalize(uni_viewPos - fragPos);
//	vec3 reflectDir = normalize(reflect(-lightDir, norm));
//	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoord));

	//　参照　Referrence/point light attenuation.png
	// 片段离光源的距离
	// フラグメントからライトまでの距離

//	float distance = length(light.lightPos - fragPos);
//	float lightFade = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

//	ambient  *= lightFade;
//	diffuse  *= lightFade;
//	specular *= lightFade;

	// 环境光照ambient
	// 環境光
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));

	// 聚光源
	// スポットライト
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);
	vec3 lightDir = normalize(light.lightPos - fragPos); //片段到spotlight的方向
														 // フラグメントからスポットライトまでの方向
	float theta = max(dot(-lightDir, normalize(light.direction)), 0.0); //spotDir与聚光源的轴方向夹角的cosine ，注意调用normalize转成单位向量
																		// spotDirとスポットライト軸方向のなす角のコサイン値（normalizeを呼び単位ベクトルに変換必須）

	// 计算光照衰减
	// 光の減衰を計算する
	float intensity = clamp((theta - light.outerCos) / (light.innerCos - light.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
    // 拡散反射光 
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * light.diffuse * vec3(texture(material.diffuse, texCoord));
	
	// 镜面光照specular
    // 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	specular = intensity * spec * light.specular * vec3(texture(material.specular, texCoord));

	// 各分量颜色混合
	// 各色成分の混合処理
	fragColor = vec4(ambient + diffuse + specular, 1.0);
}