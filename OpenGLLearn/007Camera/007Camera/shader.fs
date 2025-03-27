#version 330 core

//in vec3 color;
in vec2 texCoord;

uniform sampler2D uni_texture0;
uniform sampler2D uni_texture1;

out vec4 fragColor;

void main()
{
	fragColor = mix(texture(uni_texture0, texCoord), texture(uni_texture1, texCoord), 0.2);
}