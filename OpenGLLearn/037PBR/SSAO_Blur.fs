#version 330 core

in vec2 TexCoords;
out float FragColor;

struct Material
{
	sampler2D texture_diffuse1;  // SSAO occlusion texture
};

uniform Material material;
uniform int iSSAONoise;


void main()
{
	// 取出0维的纹理大小，也就是宽度，并以宽度构造vec2
	vec2 texelSize = 1.0 / vec2(textureSize(material.texture_diffuse1, 0));

	float result = 0.0;
	int min = -iSSAONoise / 2;
	int max = iSSAONoise / 2;
	for (int x = min; x < max; x++)
	{
		for (int y = min; y < max; y++)
		{
			vec2 offset = vec2(x, y) * texelSize;
			result += texture(material.texture_diffuse1, TexCoords + offset).r;
		}
	}

	FragColor = result / (iSSAONoise * iSSAONoise);
}
