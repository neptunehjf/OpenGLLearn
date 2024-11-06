#version 330 core

in vec2 TexCoords;
out float FragColor;

struct Material
{
	sampler2D texture_diffuse1;  // G-Buffer Position and Depth
	sampler2D texture_diffuse2;  // G-Buffer Normal
	sampler2D texture_diffuse3;  // G-Buffer Noise Texture
};

layout (std140) uniform Matrix
{
	mat4 view;
	mat4 projection;	
};

uniform Material material;
uniform int samples_num;
uniform vec3 samples[256];
uniform float window_width;
uniform float window_height;
uniform float fRadius;

// 用于放大纹理坐标，达到把原图铺满屏幕的效果
vec2 noiseScale = vec2(window_width / 4.0, window_height / 4.0);

void main()
{
	// 从G-Buffer输出的图片中取出计算SSAO所需的值
	vec3 fragPos = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 normal = texture(material.texture_diffuse2, TexCoords).rgb;
	vec3 randomVec = texture(material.texture_diffuse3, TexCoords * noiseScale).rgb;

	// 计算Tangent空间
	// 注意之前在计算法线贴图的TBN的时候，因为涉及的法线的方向，是需要按顶点对齐计算TBN的，计算方法比较复杂  
	// 而这里只需按面对齐即可，因此可以直接用Gramm-Schmidt处理，可以直接得到一个法线是normal得TBN坐标系
	// 因为randomVec是一个随机值，所以计算出的TBN是包含Noise的
	// 因为法线normal在观察空间，所以以此计算出的TBN，是基于观察空间的TBN
	// 之后切线空间的samples和randomVec 乘以 TBN，就可以转到观察空间了。
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// 计算Occlusion
	float occlusion = 0.0;
	for(int i = 0; i < samples_num; i++)
	{
		// 获取样本位置（观察空间）
		vec3 sample = TBN * samples[i]; // 切线->观察空间
		sample = fragPos + sample * fRadius; 

		// 获取样本位置（屏幕空间）
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;   // 观察->裁剪空间
		offset.xyz /= offset.w; // 透视划分（-1.0 到 1.0）
		offset.xyz = offset.xyz * 0.5 + 0.5; // 变换到0.0 - 1.0的值域

		// 与采样点比较的片段深度 (转化后的线性深度)
		float fragDepth = texture(material.texture_diffuse1, offset.xy).a;
		
		// 采样点的深度 （因为sample是观察空间的，所以也是线性深度）
		// 注意观察空间的z值是负数，所以转为正数。
		float sampleDepth = abs(sample.z);

		// 引入一个平滑插值，曲线在0.0到1.0之间
		float rangeCheck = smoothstep(0.0, 1.0, fRadius / abs(fragDepth - sampleDepth));
		// 采样点深度大于片段深度，则增加occlusion factor
		occlusion += (sampleDepth >= fragDepth ? 1.0 : 0.0) * rangeCheck;    
	}

	// 计算光照shader可以直接用的occlusion
	occlusion = 1.0 - (occlusion / samples_num);
	// 把occlusion值以图片形式输出
	FragColor = occlusion;
}
