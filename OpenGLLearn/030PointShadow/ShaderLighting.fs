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
uniform sampler2D depthMap;
uniform bool bShadow;
uniform vec3 PtLightPos; // Point Light Position
uniform samplerCube depthCubemap;
uniform float farPlane;
uniform bool bDepthCubemapDebug;
uniform float fBiasDirShadow;
uniform float fBiasPtShadow;

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

#define POINT_LIGHT_NUM 5

uniform Material material;
uniform DirectionLight dirLight;
uniform PointLight pointLight[POINT_LIGHT_NUM];
uniform SpotLight spotLight;

out vec4 fragColor;

vec4 CalcDirLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcPointLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcSpotLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcReflectionLight(vec4 reflectionColor);
float CalcDirLtShadow(vec3 norm, vec3 lightDir);
float CalcPtLtShadow();

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

	// 因为向量相加会使alpha超过1从而失去意义，所以要重新赋值
	// ベクトル加算によりアルファ値が1.0を超過すると意味がないため、再代入を実施
	color.a = diffuseColor.a;

	fragColor = color;

	if (bShadow && bDepthCubemapDebug)
	{
		vec3 lightToFrag = gs_in.fragPos - PtLightPos;
		float closestDepth = texture(depthCubemap, lightToFrag).r;
		fragColor = vec4(vec3(closestDepth / farPlane), 1.0);
	}
}

vec4 CalcDirLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// 环境光照ambient
	// 環境光
	vec4 ambient = vec4(dirLight.ambient, 1.0) * diffuseColor;

	// 漫反射光照diffuse
	// 拡散反射光 
	vec3 norm = normalize(gs_in.normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diff * vec4(dirLight.diffuse, 1.0) * diffuseColor;
	
	// 镜面光照specular
	// 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);

	float spec = 0.0f;
	// Phong
	if (light_model == 0)
	{
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	}
	// Blinn-Phong
	// 参照Referrence/blinn-phong.png
	else if (light_model == 1)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfwayDir, norm), 0.0), material.shininess);
	}

	vec4 specular = spec * vec4(dirLight.specular, 1.0) * specularColor;

	float shadow = 0.0;
	if (bShadow)
	{
		shadow = CalcDirLtShadow(norm, lightDir);
	}
		
	color = ambient + (1.0 - shadow) * (diffuse + specular);

	return color;
}

vec4 CalcPointLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 norm = normalize(gs_in.normal);
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);
	vec4 ambientTotal = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 diffuseTotal = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specularTotal = vec4(0.0, 0.0, 0.0, 1.0);

	float shadow = 0.0;

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// 环境光照ambient
		// 環境光
	    vec4 ambient = vec4(pointLight[i].ambient, 1.0) * diffuseColor;

		// 漫反射光照diffuse
		// 拡散反射光 
		vec3 lightDir = normalize(pointLight[i].lightPos - gs_in.fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse, 1.0) * diffuseColor;
	
		// 镜面光照specular
		// 鏡面反射光
		float spec = 0.0f;
		// Phong
		if (light_model == 0)
		{
			vec3 reflectDir = normalize(reflect(-lightDir, norm));
			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		}
		// Blinn-Phong
		// 参照Referrence/blinn-phong.png
		else if (light_model == 1)
		{
			vec3 halfwayDir = normalize(lightDir + viewDir);
			spec = pow(max(dot(halfwayDir, norm), 0.0), material.shininess);
		}
		vec4 specular = spec * vec4(pointLight[i].specular, 1.0) * specularColor;


	    //　参照　Referrence/point light attenuation.png		

		// 片段离光源的距离
	    // フラグメントからライトまでの距離
		float distance = length(pointLight[i].lightPos - gs_in.fragPos);
	
		float lightFade = 1.0;
		if (atte_formula == 0)
			lightFade = 1.0 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * distance * distance);
		else if (atte_formula == 1)
			lightFade = 1.0 / (0.1 * distance);
		else if (atte_formula == 2)
			lightFade = 1.0 / (0.1 * distance * distance); // 如果不启用gamma校正，lightFade经过显示器输出会变成lightFade的2.2次方，因此算法就不对了
		// 如果不启用gamma校正，lightFade经过显示器输出会变成lightFade的2.2次方，因此算法就不对了
		// 不启用gamma校正，则因为贴图自身有gamma校正也可以正常显示，但涉及到复杂算法就不一样了
		// 不启用gamma校正，相当于只有贴图gamma校正，光照算法却没有gamma校正，是错误的
		// 启用gamma校正，贴图和算法一起在最后gamma校正，是正确的
		// 说白了，就是空间转换与算法的先后问题，之前在3D空间进行矩阵计算也遇到过。
		// 是先把贴图转为非线性空间，在非线性空间进行光照算法计算，还是把贴图转成线性空间，在线性空间进行计算，最后转成非线性空间(gamma校正)

		// ガンマ補正無効時、lightFade値はディスプレイ出力で2.2乗され演算不正  
		// 補正無効でもテクスチャ側のガンマ補正で表示は正常だが複雑演算で不整合  
		// 補正無効＝テクスチャのみ補正済み（非線形空間）でライティング演算未補正（誤り）  
		// 補正有効＝テクスチャ補正解除（リニア変換）→演算→最終ガンマ補正（正解）  
		// 本質は空間変換と演算順序の問題（3D空間の行列計算と同様）  
		// 選択肢Bは正解：  
		// A) テクスチャを非線形空間変換→非線形空間で演算  
		// B) テクスチャをリニア空間変換→リニア空間で演算→非線形空間出力（ガンマ補正）  
			

		ambient  *= lightFade;
		diffuse  *= lightFade;
		specular *= lightFade;

		color += (ambient + diffuse + specular);
		ambientTotal += ambient;
		diffuseTotal += diffuse;
		specularTotal += specular;
	}

	if (bShadow)
	{
		shadow = CalcPtLtShadow();
		color = (ambientTotal + (1.0 - shadow) * (diffuseTotal + specularTotal));
	}

	return color;
}

vec4 CalcSpotLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// 环境光照ambient
	// 環境光
	vec4 ambient = vec4(spotLight.ambient, 1.0) * diffuseColor;

	// 聚光源
	// スポットライト
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 lightDir = normalize(spotLight.lightPos - gs_in.fragPos); //片段到spotlight的方向
								                             //フラグメントからスポットライトまでの方向
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir与聚光源的轴方向 ，注意调用normalize转成单位向量

	// 计算光照衰减
	// 光の減衰を計算する
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
    // 拡散反射光 
	vec3 norm = normalize(gs_in.normal);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * vec4(spotLight.diffuse, 1.0) * diffuseColor;
	
	// 镜面光照specular
    // 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);
	float spec = 0.0f;
	// Phong
	if (light_model == 0)
	{
		vec3 reflectDir = normalize(reflect(-lightDir, norm));
		spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	}
	// Blinn-Phong
	// 参照Referrence/blinn-phong.png
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
	// 反射光reflection
	vec3 I = normalize(gs_in.fragPos - uni_viewPos);
	vec3 R = normalize(reflect(I, normalize(gs_in.normal)));
	vec4 color = reflectionColor * vec4(texture(texture_cubemap1, R).rgb, 1.0);

	return color;
}

float CalcDirLtShadow(vec3 norm, vec3 lightDir)
{
	// Direction Light Shadow Mapping
	//
	// 因为归一化是在赋值glPosition才做的，这里没有经过glPosition，所以要手动归一化到[-1, 1]
	// 对正交投影没意义，因为本身就是[-1, 1]，w也是一直1；而透视投影归一化前的范围是[-w, w]，所以要除以w
	//
	// 正規化はgl_Positionへの代入時に行われるため、ここでは手動で[-1, 1]範囲に正規化
	// 平行投影の場合は元々[-1, 1]範囲であるため不要、透視投影の場合は[-w, w]範囲をwで除算

	vec3 projCoords = gs_in.fragPosLightSpace.xyz / gs_in.fragPosLightSpace.w;
	// 归一化坐标[-1, 1] 转化成 屏幕坐标[0, 1]
	// 正規化座標[-1, 1]からスクリーン座標[0, 1]へ変換
	projCoords = projCoords * 0.5 + 0.5;
	// 取得在光源视角下屏幕坐标xy位置在shadowmap对应的深度值
	// シャドウマップから光源視点での深度値を取得
	float closestDepth = texture(depthMap, projCoords.xy).r;
	// 取得当前片段在光源视角下的深度值
	// 現在のフラグメントの光源視点での深度値
	float currentDepth = projCoords.z;

	// 参照Referrence/shadow bias.png
	// bias过大，可能会导致该有阴影的地方没阴影了，最经典的就是人物的脚没有阴影，这个就是Peter-Panning现象
	// バイアスが大きすぎると、影がつくべき場所に影ができなくなり、典型的例はキャラクターの足元に影がなくなる「ピーターパン現象」
	float bias = fBiasDirShadow;

	float shadow  = 0.0;

	// 超过视锥范围视为无阴影 
	// 視錐台範囲外は影なし
	if (currentDepth > 1.0) 
		shadow  = 0.0;
	else
	{
		vec2 texelSize = 1.0 / textureSize(depthMap, 0);
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				// PCF算法 柔化阴影锯齿 // 简单的Monte Carlo采样
				// PCF（Percentage Closer Filtering）によるエイリアシング低減  簡易Monte Carloサンプリング
				float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
			}    
		}
		shadow /= 9.0;
	}

	return shadow;
}

float CalcPtLtShadow()
{
	// Point Light Shadow Mapping
	vec3 lightToFrag = gs_in.fragPos - PtLightPos;

	// 要渲染的片段的深度
	// レンダリングするフラグメントの深度
	float currentDepth = length(lightToFrag);

	float bias = fBiasPtShadow;
	float shadow = 0.0;
	int samples = 20;
	float offset = 0.1;
	float radius = 0.05;

	vec3 sampleOffsetDirections[20] = vec3[]
	(
	   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);

	// 超过视锥范围视为无阴影 
	// 視錐台範囲外は影なし
	if (currentDepth > 1.0 * farPlane)
		shadow = 0.0;
	else
	{
		// 確率的サンプリングによるソフトシャドウ生成
		for (int i = 0; i < samples; i++)
		{
			// 离光源最近的片段的深度
			// ポイントライトから最近接深度を取得
			float closestDepth = texture(depthCubemap, lightToFrag + sampleOffsetDirections[i] * radius).r;
			// 从[0,1]的范围转化成原来的范围
			// [0,1]正規化値を実深度値に変換
			closestDepth *= farPlane;
			if (currentDepth - bias > closestDepth)
				shadow += 1.0;
		}

		shadow /= samples;
	}

	return shadow;
}