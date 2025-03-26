#version 330 core

in vec4 vertexColor; // 当前shader的输入属性必须和上一个输出属性的类型，名称相同，否则不能链接起来    
					 // 現在のシェーダーの入力属性は前段シェーダーの出力属性と型/名前が完全一致しないとリンク不可

out vec4 fragColor;


void main()
{
	fragColor = vertexColor;
}