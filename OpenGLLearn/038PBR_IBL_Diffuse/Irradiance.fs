#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube texture_cubemap1; // 环境cubemap

const float PI = 3.14159265359;

vec3 convolution(vec3 normal);

void main()
{
	// 片段的法线方向作为采样方向
	vec3 normal = normalize(localPos);

	vec3 irradiance = convolution(normal);

	FragColor = vec4(irradiance, 1.0);
}

// 用卷积算出片段的反射光
vec3 convolution(vec3 normal)
{
	vec3 irradiance = vec3(0.0);
	
	// 以normal为z轴构造一个正交坐标系
	// 因为normal是一个局部空间的坐标，所以以normal构造的坐标系是一个局部空间的坐标系
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal)); // up初始值是什么无所谓，这里都能算出一个和normal垂直的值
	up = normalize(cross(normal, right));
	mat3 TBN = mat3(right, up, normal);

	// 在反射光的方向（采样方向）的半球内，均匀选取若干离散入射光，然后计算各自的平均值（黎曼和）
	// 半球 航向角phi:0~2PI  极角theta:0~0.5PI
	int sampleNum = 0;
	float sampleDelta;

	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// tangent空间 球面坐标 -》笛卡尔坐标
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent空间 -》local空间
			vec3 sample = tangentSample * TBN;

			irradiance += texture(texture_cubemap1, sample).rgb * cos(theta) * sin(theta); 
			sampleNum++;
		}
	}

	// c 和 Kd 放到对应其物理学意义的地方计算，这里暂且不计算（实际效果没区别）
	irradiance = PI * irradiance * (1.0 / (float(sampleNum))); 
	
	return irradiance;
}