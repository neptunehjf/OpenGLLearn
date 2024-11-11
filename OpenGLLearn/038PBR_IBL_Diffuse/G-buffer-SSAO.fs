#version 330 core

layout(location = 0) out vec4 gPositionDepth;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform float near; // 投影矩阵的近平面
uniform float far; // 投影矩阵的远平面

// 转化成线性空间（观察空间）
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // 回到NDC
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
	gPositionDepth.rgb = FragPos;
	gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);

	gNormal = normalize(Normal); 

	// 材质设置成白色，可以更容易看出SSAO效果
	gAlbedo = vec3(0.95);
}