#version 330 core

// 输出两个颜色缓冲，一个原图，一个bloom用的亮色图
// 2つのカラーバッファを出力：オリジナル画像用 と ブルーム用高輝度マップ
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in GS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;
	mat3 TBN;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
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
uniform bool bNormalMap;   // 当前片段是否应用了法线贴图 
			   // 法線マップを適用するか
uniform bool bParallaxMap; // 当前片段是否应用了视差贴图
			   // 視差マップを適用するか
uniform float height_scale;
uniform int iParaAlgo; // 视差采样算法类型
                       // 視差サンプリングのアルゴリズムタイプ

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflection1;
	sampler2D texture_normal1;
	sampler2D texture_disp1;
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


vec4 CalcDirLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcPointLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcSpotLight(vec4 diffuseColor, vec4 specularColor);
vec4 CalcReflectionLight(vec4 reflectionColor);
float CalcDirLtShadow(vec3 norm, vec3 lightDir);
float CalcPtLtShadow();
vec3 GetNormalFromTexture();
vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir);
vec2 ParallaxMappingSimple(vec2 texCoord, vec3 viewDir);
vec2 ParallaxMappingSteep(vec2 texCoord, vec3 viewDir);
vec2 ParallaxMappingOcclusion(vec2 texCoord, vec3 viewDir);

float near = 0.1; 
float far  = 100.0; 
vec2 texCoord = gs_in.texCoord;

void main()
{
	if (bParallaxMap)
	{
		vec3 tangentViewDir = normalize(gs_in.TangentViewPos - gs_in.TangentFragPos);
		texCoord = ParallaxMapping(gs_in.texCoord, tangentViewDir);
		if(texCoord.x > 5.0 || texCoord.y > 5.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
			discard;
	}

	vec4 diffuseColor = texture(material.texture_diffuse1, texCoord);
	vec4 specularColor = texture(material.texture_specular1, texCoord);
	vec4 reflectionColor = texture(material.texture_reflection1, texCoord);

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	color += CalcDirLight(diffuseColor, specularColor);
	color += CalcPointLight(diffuseColor, specularColor);
	color += CalcSpotLight(diffuseColor, specularColor);
	color += CalcReflectionLight(reflectionColor);

	// 因为向量相加会使alpha超过1从而失去意义，所以要重新计算
	color.a = diffuseColor.a;

	FragColor = color;

	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
	    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	if (bShadow && bDepthCubemapDebug)
	{
		vec3 lightToFrag = gs_in.fragPos - PtLightPos;
		float closestDepth = texture(depthCubemap, lightToFrag).r;
		FragColor = vec4(vec3(closestDepth / farPlane), 1.0);
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
	vec3 norm = GetNormalFromTexture();

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
	vec3 viewDir = normalize(uni_viewPos - gs_in.fragPos);
	vec4 ambientTotal = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 diffuseTotal = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specularTotal = vec4(0.0, 0.0, 0.0, 1.0);

	float shadow = 0.0;

	vec3 norm = GetNormalFromTexture();

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
	vec3 norm = GetNormalFromTexture();

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

vec3 GetNormalFromTexture()
{
	vec3 norm;
	// 如果有法线贴图，则用法线贴图的法线
	// 法線マップがある場合は法線マップから取得
	if (bNormalMap)
	{
		norm = texture(material.texture_normal1, texCoord).rgb;
		// 法线贴图的法线是以RGB形式存储的，每个分量范围是[0, 1], 要转成[-1, 1]的形式（因为单位向量的各分量的范围是[-1, 1]）
		// 法線マップの値は[0,1]範囲のRGB形式のため[-1,1]範囲に変換（単位ベクトルの成分範囲対応）
		norm = normalize(2 * (norm - 0.5));
		norm = normalize(gs_in.TBN * norm);
	}
	// 如果没有有法线贴图，则用顶点数据的法线
	// 法線マップがない場合は頂点法線を使用
	else
		norm = normalize(gs_in.normal);

	return norm;
}

vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir)
{
	if (iParaAlgo == 0)
		return ParallaxMappingSimple(texCoord, viewDir);
	else if (iParaAlgo == 1)
		return ParallaxMappingSteep(texCoord, viewDir);
	else if (iParaAlgo == 2)
		return ParallaxMappingOcclusion(texCoord, viewDir);
}

// 参照Referrence/Parallax Mapping Simple.png
vec2 ParallaxMappingSimple(vec2 texCoord, vec3 viewDir)
{
	float height = texture(material.texture_disp1, texCoord).r;

	// viewDir.xy / viewDir.z 是x和y方向分别与z方向的夹角的cos值
	// height 是fragPos距离dispMap的深度的垂直距离
	// height_scale 提供一些额外的控制
	// p 是 实际采样的纹理坐标与原texCoord的偏移

	// viewDir.xy / viewDir.z はx/y方向とz方向の角度のcos値
    // height はフラグメント位置とディスプレースメントマップの垂直距離
    // height_scale で効果強度を調整
    // p は実際のサンプリング座標のオフセット量
	vec2 p = (viewDir.xy / viewDir.z) * height * height_scale;

	// 计算偏移后的纹理坐标
	// 因为计算出来的p方向和viewDir方向一致，实际上偏移方向应该和视角方向相反，所以是减法
	// 視点方向と逆方向に座標をシフト
	return texCoord - p;
}

// 参照Referrence/Parallax Steep Mapping.png
vec2 ParallaxMappingSteep(vec2 texCoord, vec3 viewDir)
{
	// currentLayer > currentDepth  return texCoord - delta

	// 总深度层级数，越大采样越多，越精确
	// 総深度レイヤー数（大きいほど高精度）
	float layerNum = 10.0;

	// 每层的深度差
	// レイヤー間の深度差
	float deltaLayer = 1.0  / layerNum;

	// 采样的纹理坐标的总范围。实际viewDir是单位向量，所以xy只表示方向，长度无意义。height_scale用来代表长度，可以debug出一个合适的值
	//  テクスチャ座標の総変位量（viewDirは単位ベクトルのためxy成分は方向のみ、height_scaleで実効長を調整）
	vec2 p = viewDir.xy * height_scale;

	// 每层的采样的纹理坐标的差
	// レイヤー毎のテクスチャ座標変化量
	vec2 deltaTexCoord = p / layerNum;

	// 当前采样的层级
	// 現在のサンプリングレイヤー
	float currentLayer = 0.0;

	// 当前采样的深度
	// 現在の深度値（高さ情報）
	float currentDepth = texture(material.texture_disp1, texCoord).r;

	// 当前采样的纹理坐标
	// 現在のテクスチャ座標
	vec2 currentTexCoord = texCoord;

	// 找到层级大于深度的采样点
	// レイヤー値が深度値を超える点を探索
	while(currentLayer < currentDepth)
	{
		currentLayer += deltaLayer;
		currentTexCoord -= deltaTexCoord; // 最上层的采样点不可能满足条件，所以不采样也可以
	//									　// 最上層座標は条件を満たさないためスキップ可能
		currentDepth = texture(material.texture_disp1, currentTexCoord).r;
	}

	return currentTexCoord;
}

// 参照Referrence/Parallax Occlusion Mapping.png
vec2 ParallaxMappingOcclusion(vec2 texCoord, vec3 viewDir)
{
	// 求深度图上临界的两个点的连线 与 视线的交叉点，插值采样
	// 深度マップ上で隣接する2点の補間によるオクルージョン処理

	// 总深度层级数，越大采样越多，越精确
	// 総深度レイヤー数（大きいほど高精度）
	float layerNum = 10.0;

	// 每层的深度差
	// レイヤー間の深度差
	float deltaLayer = 1.0  / layerNum;

	// 采样的纹理坐标的总范围。实际viewDir是单位向量，所以xy只表示方向，长度无意义。height_scale用来代表长度，可以debug出一个合适的值
	//  テクスチャ座標の総変位量（viewDirは単位ベクトルのためxy成分は方向のみ、height_scaleで実効長を調整）
	vec2 p = viewDir.xy * height_scale;

	// 每层的采样的纹理坐标的差
	// レイヤー毎のテクスチャ座標変化量
	vec2 deltaTexCoord = p / layerNum;

	// 当前采样的层级
	// 現在のサンプリングレイヤー
	float currentLayer = 0.0;

	// 当前采样的深度
	// 現在の深度値（高さ情報）
	float currentDepth = texture(material.texture_disp1, texCoord).r;

	// 当前采样的纹理坐标
	// 現在のテクスチャ座標
	vec2 currentTexCoord = texCoord;

	// 找到层级大于深度的采样点
	// レイヤー値が深度値を超える点を探索
	while(currentLayer < currentDepth)
	{
		currentLayer += deltaLayer;
		currentTexCoord -= deltaTexCoord; // 最上层的采样点不可能满足条件，所以不采样也可以
	//									　// 最上層座標は条件を満たさないためスキップ可能
		currentDepth = texture(material.texture_disp1, currentTexCoord).r;
	}

	// 求临界前一点的纹理坐标
	// 境界直前のテクスチャ座標
	vec2 beforeTexCoord = currentTexCoord + deltaTexCoord;

	// 境界前後の深度差分を計算
	float beforeLayer = currentLayer - deltaLayer;
	float beforeDiff = abs(beforeLayer - texture(material.texture_disp1, beforeTexCoord).r);
	float afterDiff = abs(currentLayer - currentDepth);

	// 根据两个三角形的比例关系算出
	// 線形補間による最終座標計算
	vec2 finalTexCoord = currentTexCoord * (beforeDiff / (beforeDiff + afterDiff)) + beforeTexCoord * (afterDiff / (beforeDiff + afterDiff));

	return finalTexCoord;
}
