#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec3 lightPos;

uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;

uniform float uni_ambientStrength;
uniform float uni_diffuseStrength;
uniform float uni_specularStrength;
uniform int uni_specularFactor;

out vec4 fragColor;

void main()
{
	// ��������ambient
	vec3 ambient = uni_ambientStrength * uni_lightColor;
	
	// ���������diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = uni_diffuseStrength * diff * uni_lightColor;
	
	// �������specular
	vec3 viewDir = normalize(vec3(0.0, 0.0, 0.0) - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), uni_specularFactor);
	vec3 specular = uni_specularStrength * spec * uni_lightColor;

	// ������ɫ��������ɫ��ϣ�Ӫ�����Ч��
	fragColor = vec4((ambient + diffuse + specular) * uni_objectColor, 1.0);

}