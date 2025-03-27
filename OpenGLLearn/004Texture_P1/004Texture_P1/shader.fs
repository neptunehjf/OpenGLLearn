#version 330 core

in vec3 color;
in vec2 texCoord;

uniform sampler2D uni_texture0;
uniform sampler2D uni_texture1;
uniform float uni_mixValue;

out vec4 fragColor;

void main()
{
	//texCoord.x = -texCoord.x; 注意改变输入值会编译不过，估计是因为输入是只读的
	//	警告：入力値変更によるコンパイルエラー検出。原因はreadonly属性付きシェーダー入力バッファへの不正書き込み試行と推定					
	
	fragColor = mix(texture(uni_texture0, texCoord), texture(uni_texture1, vec2(texCoord.x, texCoord.y)), uni_mixValue);
}