#version 330 core

// fragment shader的输入变量都是经过光栅化插值的
// フラグメントシェーダーの入力変数は全てラスタライザによる補間処理を経ている
in vec3 fragPos;
in vec3 normal;
in vec3 lightPos;
in vec4 fragColor;

out vec4 color;

void main()
{
	color = fragColor;
}