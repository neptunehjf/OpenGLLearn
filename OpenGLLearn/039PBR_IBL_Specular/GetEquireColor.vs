#version 330 core
layout(location = 0) in vec3 aPos;
// 局部空间的aPos等于aNormal，所以不需要aNormal数据

// 和点光源阴影一样，需要在特定位置以6个视角来绘制，只不过点光源是以光源视角，这里是以世界原点视角	
uniform mat4 transforms;  // 记录当前面的transforms， 也就是 projection * view

out vec3 localPos;

void main()
{
	// 在局部空间计算即可，所以不需要model矩阵
	localPos = aPos;
	gl_Position =  transforms * vec4(localPos, 1.0);
}