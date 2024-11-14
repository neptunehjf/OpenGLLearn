#version 330 core

in vec3 TexCoords;

uniform samplerCube texture_cubemap1;

out vec4 FragColor;

void main()
{
	vec3 color = texture(texture_cubemap1, TexCoords).rgb;

	// HDR tonemap
	// 放到postprocess shader里处理

	// Gamma Correct 因为已经在CPU侧设置了glEnable(GL_FRAMEBUFFER_SRGB); 下面代码不需要
    // color = pow(color, vec3(1.0/2.2)); 

	FragColor = vec4(color, 1.0);
}