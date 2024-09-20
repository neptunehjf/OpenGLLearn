#version 330 core

in GS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;
	mat3 TBN;
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
uniform bool bNormalMap;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflection1;
	sampler2D texture_normal1;
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
vec3 GetNormal();

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

	// 因为向量相加会使alpha超过1从而失去意义，所以要重新计算
	color.a = diffuseColor.a;

	// 各分量颜色混合
	fragColor = color;

	if (bShadow && bDepthCubemapDebug)
	{
		vec3 lightToFrag = gs_in.fragPos - PtLightPos;
		float closestDepth = texture(depthCubemap, lightToFrag).r;
		fragColor = vec4(vec3(closestDepth / farPlane), 1.0);
	}

	// debug
	//vec3 norm;
	//if (bNormalMap)
	//{
	//	norm = texture(material.texture_normal1, gs_in.texCoord).rgb;
	//	fragColor = vec4(norm, 1.0);
	//}
}

vec4 CalcDirLight(vec4 diffuseColor, vec4 specularColor)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// 环境光照ambient
	vec4 ambient = vec4(dirLight.ambient, 1.0) * diffuseColor;

	// 漫反射光照diffuse
	vec3 norm = GetNormal();

	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diff * vec4(dirLight.diffuse, 1.0) * diffuseColor;
	
	// 镜面光照specular
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

	vec3 norm = GetNormal();

	for (int i = 0; i < POINT_LIGHT_NUM; i++)
	{
		// 环境光照ambient
	    vec4 ambient = vec4(pointLight[i].ambient, 1.0) * diffuseColor;

		// 漫反射光照diffuse
		vec3 lightDir = normalize(pointLight[i].lightPos - gs_in.fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = diff * vec4(pointLight[i].diffuse, 1.0) * diffuseColor;
	
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
		float distance = length(pointLight[i].lightPos - gs_in.fragPos);
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
	vec4 ambient = vec4(spotLight.ambient, 1.0) * diffuseColor;

	// 聚光源
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 lightDir = normalize(spotLight.lightPos - gs_in.fragPos); //片段到spotlight的方向
	float theta = max(dot(-lightDir, normalize(spotLight.direction)), 0.0); //spotDir与聚光源的轴方向 ，注意调用normalize转成单位向量

	// 计算边缘的光照衰减
	float intensity = clamp((theta - spotLight.outerCos) / (spotLight.innerCos - spotLight.outerCos), 0.0, 1.0); //用clamp就不需要ifelse了

	// 漫反射光照diffuse
	vec3 norm = GetNormal();

	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = intensity * diff * vec4(spotLight.diffuse, 1.0) * diffuseColor;
	
	// 镜面光照specular
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
	vec3 projCoords = gs_in.fragPosLightSpace.xyz / gs_in.fragPosLightSpace.w;
	// 归一化坐标[-1, 1] 转化成 屏幕坐标[0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// 取得在光源视角下屏幕坐标xy位置在depthMap对应的深度值
	float closestDepth = texture(depthMap, projCoords.xy).r;
	// 取得当前片段在光源视角下的深度值
	float currentDepth = projCoords.z;

	float bias = fBiasDirShadow; // bias过大，可能会导致该有阴影的地方没阴影了，最经典的就是人物的脚没有阴影，这个就是Peter-Panning现象
	float shadow  = 0.0;
	if (currentDepth > 1.0) // 超过视锥范围视为无阴影
		shadow  = 0.0;
	else
	{
		vec2 texelSize = 1.0 / textureSize(depthMap, 0);
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				// PCF算法 柔化阴影锯齿
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

	if (currentDepth > 1.0 * farPlane) // 超过视锥范围视为无阴影
		shadow = 0.0;
	else
	{
		for (int i = 0; i < samples; i++)
		{
			// 离光源最近的片段的深度
			float closestDepth = texture(depthCubemap, lightToFrag + sampleOffsetDirections[i] * radius).r;
			// 从[0,1]的范围转化成原来的范围
			closestDepth *= farPlane;
			if (currentDepth - bias > closestDepth)
				shadow += 1.0;
		}

		shadow /= samples;
	}

	return shadow;
}

vec3 GetNormal()
{
	vec3 norm;
	// 如果有法线贴图，则用法线贴图的法线
	if (bNormalMap)
	{
		norm = texture(material.texture_normal1, gs_in.texCoord).rgb;
		// 法线贴图的法线是以RGB形式存储的，每个分量范围是[0, 1], 要转成[-1, 1]的形式（因为单位向量的各分量的范围是[-1, 1]）
		norm = normalize(2 * (norm - 0.5)); //不用忘记normalize
		norm = normalize(gs_in.TBN * norm);
	}
	// 如果没有有法线贴图，则用顶点数据的法线
	else
		norm = normalize(gs_in.normal);

	return norm;
}