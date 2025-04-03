#version 330 core

in vec2 TexCoords;
in mat4 Projection;
out float FragColor;

struct Material
{
	sampler2D texture_diffuse1;  // G-Buffer Position and Depth
	sampler2D texture_diffuse2;  // G-Buffer Normal
	sampler2D texture_diffuse3;  // G-Buffer Noise Texture
};

uniform Material material;
uniform int samples_num;
uniform vec3 samples[256];
uniform float window_width;
uniform float window_height;
uniform float fRadius;
uniform int iSSAONoise;

// 用于放大纹理坐标，达到把原图铺满屏幕的效果
// テクスチャ座標を拡大し、元画像を画面全体に繰り返しで表示する効果を実現
vec2 noiseScale = vec2(window_width / iSSAONoise, window_height / iSSAONoise);

void main()
{
	// 从G-Buffer输出的图片中取出计算SSAO所需的值
	// GバッファからSSAO計算に必要な値を取得
	vec3 fragPos = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 normal = texture(material.texture_diffuse2, TexCoords).rgb;
	vec3 randomVec = texture(material.texture_diffuse3, TexCoords * noiseScale).rgb;

	// 计算Tangent空间
	// 注意之前在计算法线贴图的TBN的时候，因为涉及的法线的方向，是需要按顶点对齐计算TBN的，计算方法比较复杂  
	// 但是这里不要精确到按顶点对齐，因此可以直接用Gramm-Schmidt处理，可以直接得到一个法线是normal的TBN坐标系
	// 因为randomVec是一个随机值，所以计算出的TBN是包含Noise的
	// 因为法线normal在观察空间，所以以此计算出的TBN，是基于观察空间的TBN
	// 之后切线空间的samples和randomVec 乘以 TBN，就可以转到观察空间了。
	//
	// 接空間(Tangent Space)の計算
    // 法線マップ用TBN計算では頂点単位の正確な計算が必要だが、
    // ここではGramm-Schmidt処理で簡易的に法線normalに基づくTBN座標系を生成
    // ノイズを含むrandomVecによりTBNにランダム性を導入
    // 観測空間(view space)ベースの座標系

	// Gramm-Schmidt 再正交化。求出与normal垂直并且受randomVec影响的tangent
	// Gramm-Schmidt直交化：normalと直交するtangentを生成
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));

	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Occlusion计算
	float occlusion = 0.0;
	for(int i = 0; i < samples_num; i++)
	{
		// 获取样本位置 切线->观察空间
		// サンプル位置 接空間→観測空間
		vec3 sample = TBN * samples[i];
		sample = fragPos + sample * fRadius; 

		// 获取样本位置（屏幕空间）
		// スクリーン空間への投影
		vec4 offset = vec4(sample, 1.0);
		// 观察->裁剪空间  
		// 観測→クリップ空間
		offset = Projection  * offset;  
		 // 透视划分（-1.0 到 1.0）
		 // パースペクティブ除算(-1.0～1.0)
		offset.xyz /= offset.w;
		// 变换到0.0 - 1.0的值域
		// 0.0～1.0範囲に変換
		offset.xyz = offset.xyz * 0.5 + 0.5; 

		// 与采样点比较的片段深度 (转化后的线性深度)
		// 比較対象フラグメントの深度値(線形深度)
		float fragDepth = texture(material.texture_diffuse1, offset.xy).a;
		
		// 采样点的深度 （因为sample是观察空间的，所以也是线性深度）
		// 注意观察空间的z值是负数，所以转为正数。
		 // サンプル点の深度値(観測空間のため　線形深度)(観測空間のz値は負数のため絶対値)
		float sampleDepth = abs(sample.z);

		// 边缘检测，排除深度差在采样范围外的情形
		// エッジ検出：深度差がサンプリング範囲外の場合をフィルタリング
		// 
		// 引入一个平滑插值，曲线在0.0到1.0之间，过渡效果更自然
		// 0.0～1.0のsmoothstep補間で自然な境界遷移を実現
		float rangeCheck = smoothstep(0.0, 1.0, fRadius / abs(fragDepth - sampleDepth));

		// 参照Referrence/SSAO sample.png
		// 采样点深度大于片段深度，则增加occlusion factor
		occlusion += (sampleDepth >= fragDepth ? 1.0 : 0.0) * rangeCheck;    
	}

	 // 最終オクルージョン値の正規化
	occlusion = 1.0 - (occlusion / samples_num);

	FragColor = occlusion;
}
