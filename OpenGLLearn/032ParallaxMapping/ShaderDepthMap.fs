#version 330 core

void main()
{
	// 不写这一行默认也会执行
	// この行を書かない場合、デフォルトで同じ処理が実行されます
	gl_FragDepth = gl_FragCoord.z; 

}