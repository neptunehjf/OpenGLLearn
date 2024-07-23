// 虽然在shader.fs用if else语句也能分别渲染光源和物体的颜色，但是单独写一个shader有它的好处

#version 330 core

out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0);
}