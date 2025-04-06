#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // 環境cubemap

const float PI = 3.14159265359;

vec3 convolution(vec3 normal);

void main()
{
	// 片段的法线方向作为采样方向
	// フラグメントの法線方向をサンプリング方向とする
	vec3 normal = normalize(localPos);

	vec3 irradiance = convolution(normal);

	FragColor = vec4(irradiance, 1.0);
}

// 用卷积算出片段的反射光
// 畳み込み計算によるフラグメントの反射光算出
vec3 convolution(vec3 normal)
{
	vec3 irradiance = vec3(0.0);
	
	// 以normal为z轴构造一个正交坐标系
	// 因为normal是一个局部空间的坐标，所以以normal构造的坐标系是一个局部空间的坐标系
	//
	// normalをz軸とする直交座標系を構築
    // normalはローカル空間座標であるため、これから構築する座標系はローカル空間座標系となる

	// 因为是半球采样，up和right指向哪里都可以
	// 半球サンプリングの場合、upとrightの向きはどこでも問題ありません。
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal)); 								
	up = normalize(cross(normal, right));
	mat3 TBN = mat3(right, up, normal);

	// 在反射光的方向（采样方向）的半球内，均匀选取若干离散入射光，然后计算各自的平均值（黎曼和）
	// 半球 航向角phi:0~2PI  极角theta:0~0.5PI
	//
	// 反射方向（サンプリング方向）の半球内で離散入射光を一様サンプリングし平均値（リーマン和）を計算
    // 半球座標系 方位角phi:0～2π 極角theta:0～0.5π
	int sampleNum = 0;
	float sampleDelta = 0.025;

	// 参照 Referrence/spherical_to_cartesian.jpg
	// 参照 Referrence/solid angle integral.jpg
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// tangent空间 球面坐标 -》笛卡尔坐标
			// 接空間：球面座標 → デカルト座標 
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			// tangent空间 -》local空间
			// 接空間 → ローカル空間
			vec3 sample = tangentSample * TBN;
	
			irradiance += texture(texture_cubemap1, sample).rgb * cos(theta) * sin(theta); 
			sampleNum++;
		}
	}

	// c 和 Kd 放到对应其物理学意义的地方计算，这里暂且不计算（实际效果没区别）
	// cとKdの計算は物理的な意味がある箇所で実施（実際の効果に差異なし）
	irradiance = PI * irradiance * (1.0 / (float(sampleNum))); 
	
	return irradiance;
}