#version 330 core

in vec4 vertexColor; //当前shadr的输入属性必须和上一个输出属性的类型，名称相同，否则不能链接起来    
out vec4 fragColor;


void main()
{
	fragColor = vertexColor;
}