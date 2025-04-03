#version 330 core

layout(location = 0) out vec4 gPositionDepth;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform float near; // 射影行列のニアクリップ面
uniform float far;  // 射影行列のファークリップ面

// 深度値の線形化（　=> view space）
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // NDC正規化（-1～1範囲）
    return (2.0 * near * far) / (far + near - z * (far - near)); // 逆射影計算
}

void main()
{
	gPositionDepth.rgb = FragPos;
	gPositionDepth.a = LinearizeDepth(gl_FragCoord.z);

	gNormal = normalize(Normal); 

// アルベド値（マテリアルを白色に設定　SSAO効果の確認用）
	gAlbedo = vec3(0.95);
}