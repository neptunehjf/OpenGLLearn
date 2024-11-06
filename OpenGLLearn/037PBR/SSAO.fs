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

// ���ڷŴ��������꣬�ﵽ��ԭͼ������Ļ��Ч��
vec2 noiseScale = vec2(window_width / 4.0, window_height / 4.0);

void main()
{
	// ��G-Buffer�����ͼƬ��ȡ������SSAO�����ֵ
	vec3 fragPos = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 normal = texture(material.texture_diffuse2, TexCoords).rgb;
	vec3 randomVec = texture(material.texture_diffuse3, TexCoords * noiseScale).rgb;

	// ����Tangent�ռ�
	// ע��֮ǰ�ڼ��㷨����ͼ��TBN��ʱ����Ϊ�漰�ķ��ߵķ�������Ҫ������������TBN�ģ����㷽���Ƚϸ���  
	// ������ֻ�谴����뼴�ɣ���˿���ֱ����Gramm-Schmidt��������ֱ�ӵõ�һ��������normal��TBN����ϵ
	// ��ΪrandomVec��һ�����ֵ�����Լ������TBN�ǰ���Noise��
	// ��Ϊ����normal�ڹ۲�ռ䣬�����Դ˼������TBN���ǻ��ڹ۲�ռ��TBN
	// ֮�����߿ռ��samples��randomVec ���� TBN���Ϳ���ת���۲�ռ��ˡ�
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// ����Occlusion
	float occlusion = 0.0;
	for(int i = 0; i < samples_num; i++)
	{
		// ��ȡ����λ�ã��۲�ռ䣩
		vec3 sample = TBN * samples[i]; // ����->�۲�ռ�
		sample = fragPos + sample * fRadius; 

		// ��ȡ����λ�ã���Ļ�ռ䣩
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;   // �۲�->�ü��ռ�
		offset.xyz /= offset.w; // ͸�ӻ��֣�-1.0 �� 1.0��
		offset.xyz = offset.xyz * 0.5 + 0.5; // �任��0.0 - 1.0��ֵ��

		// �������Ƚϵ�Ƭ����� (ת������������)
		float fragDepth = texture(material.texture_diffuse1, offset.xy).a;
		
		// ���������� ����Ϊsample�ǹ۲�ռ�ģ�����Ҳ��������ȣ�
		// ע��۲�ռ��zֵ�Ǹ���������תΪ������
		float sampleDepth = abs(sample.z);

		// ����һ��ƽ����ֵ��������0.0��1.0֮��
		float rangeCheck = smoothstep(0.0, 1.0, fRadius / abs(fragDepth - sampleDepth));
		// ��������ȴ���Ƭ����ȣ�������occlusion factor
		occlusion += (sampleDepth >= fragDepth ? 1.0 : 0.0) * rangeCheck;    
	}

	// �������shader����ֱ���õ�occlusion
	occlusion = 1.0 - (occlusion / samples_num);
	// ��occlusionֵ��ͼƬ��ʽ���
	FragColor = occlusion;
}
