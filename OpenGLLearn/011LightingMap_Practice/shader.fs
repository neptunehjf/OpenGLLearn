#version 330 core

// fragment shader的输入变量都是经过光栅化插值的
// フラグメントシェーダーの入力変数は全てラスタライザによる補間処理を経ている
in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform vec3 uni_viewPos;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	int shininess;
	sampler2D glow;
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
	// 環境光
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	
	// 漫反射光照diffuse
    // 拡散反射光 
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, texCoord));
	
	// 镜面光照specular
    // 鏡面反射光
	vec3 viewDir = normalize(uni_viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoord));
	//vec3 specular = spec * light.specular * (vec3(1.0) - vec3(texture(material.specular, texCoord)));

	// 物体自发光 glow
	// 自発光する物体
	vec3 glow = texture(material.glow, texCoord).rgb;

	// 光照颜色与物体颜色混合，营造光照效果
    // 光の色と物体の色をブレンド
	fragColor = vec4(ambient + diffuse + specular + glow, 1.0);
}