#version 330 core

in vec3 fragPos;
in vec3 normalModel;
in vec3 normalView;

uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;
uniform vec3 uni_lightPos;
uniform vec3 uni_viewPos;

out vec4 fragColor;

void main()
{
	// 环境光照ambient
	float ambientStrength = 0.2;
	vec3 ambient = ambientStrength * uni_lightColor;
	
	// 漫反射光照diffuse
	vec3 nm = normalize(normalModel);
	vec3 lightDir = normalize(uni_lightPos - fragPos);
	float diff = max(dot(nm, lightDir), 0.0);
	vec3 diffuse = diff * uni_lightColor;
	
	// 镜面光照specular
	float specularStrength = 0.5;
	vec3 nv = normalize(normalView);
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, nv));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * uni_lightColor;

	// 光照颜色与物体颜色混合，营造光照效果
	fragColor = vec4((ambient + diffuse + specular) * uni_objectColor, 1.0);

}