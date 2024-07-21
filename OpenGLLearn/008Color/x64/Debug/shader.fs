#version 330 core

uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;
//uniform bool uni_isLight;  // 虽然在shader.fs用if else语句也能分别渲染光源和物体的颜色，但是单独写一个shader有它的好处

out vec4 fragColor;

void main()
{
	fragColor = vec4(uni_objectColor * uni_lightColor, 1.0);
}