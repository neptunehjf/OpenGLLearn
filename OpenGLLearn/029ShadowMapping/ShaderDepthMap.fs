#version 330 core

void main()
{
	gl_FragDepth = gl_FragCoord.z; //不写这一行默认也会执行
}