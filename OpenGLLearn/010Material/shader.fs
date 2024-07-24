#version 330 core

in vec3 fragPos;
in vec3 normalModel;
in vec3 normalView;

uniform vec3 uni_viewPos;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	int shininess;
};

struct Light
{
	vec3 lightPos;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

out vec4 fragColor;

void main()
{
	// 环境光照ambient
	vec3 ambient = material.ambient * light.ambient;
	
	// 漫反射光照diffuse
	vec3 norm = normalize(normalModel);
	vec3 lightDir = normalize(light.lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = material.diffuse * diff * light.diffuse;
	
	// 镜面光照specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = material.specular * spec * light.specular;

	// 光照颜色与物体颜色混合，营造光照效果
	fragColor = vec4(ambient + diffuse + specular, 1.0);

}