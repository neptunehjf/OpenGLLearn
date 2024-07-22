#version 330 core

in vec3 normal;
in vec3 fragPos;

uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;
uniform vec3 uni_lightPos;

out vec4 fragColor;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * uni_lightColor;
	
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(uni_lightPos - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * uni_lightColor;
	
	fragColor = vec4((ambient + diffuse)* uni_objectColor, 1.0);

}