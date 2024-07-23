#version 330 core

in vec3 fragPos;
in vec3 normalModel;
in vec3 normalView;

uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;
uniform vec3 uni_lightPos;
uniform vec3 uni_viewPos;

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
	vec3 norm = normalize(normalModel);
	vec3 lightDir = normalize(uni_lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = uni_diffuseStrength * diff * uni_lightColor;
	
	// �������specular
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), uni_specularFactor);
	vec3 specular = uni_specularStrength * spec * uni_lightColor;

	// ������ɫ��������ɫ��ϣ�Ӫ�����Ч��
	fragColor = vec4((ambient + diffuse + specular) * uni_objectColor, 1.0);

}