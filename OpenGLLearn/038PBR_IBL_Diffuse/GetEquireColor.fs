#version 330 core
out vec4 FragColor;

in vec3 localPos;

struct Material
{
	sampler2D texture_diffuse1; // 等距圆柱投影图
};

uniform Material material;

// 根据Cube的片段位置算出对应的等距圆柱投影图的UV坐标
vec2 EquirectangularUV(vec3 v)
{
	// 笛卡尔坐标 to 球面坐标
	float theta = asin(v.y); // 注意，这里的theta并不是球面坐标定义极角，而是与xy平面的夹角
	float phi = atan(v.z, v.x); // 航向角，此处定义为与x轴的夹角（也可以定义为与Z轴的夹角，影响不大，看具体怎样实现了）
	
	// 球面坐标 to 等距圆柱投影图的UV坐标
	vec2 uv = vec2(phi, theta); // 因为phi是航向角，展开球体后对应水平方向；theta是极角的补角，展开球体后对应垂直方向
	
	// 标准化UV坐标
	// 用常数vec2(0.1591, 0.3183)把UV坐标标准化到 (0,1)的范围
	// 显然球体展开成等距圆柱投影图，y轴尺度不变，而水平方向的尺度由于x维度和z维度的叠加，尺度加倍了，所以标准化常数的水平分量正好是垂直分量的一半
	const vec2 invAtan = vec2(0.1591, 0.3183);
	uv *= invAtan;
    uv += 0.5;

	return uv;
}

void main()
{
	//localPos必须normalize，因为上述计算都是假定球体半径为1的
	vec2 uv = EquirectangularUV(normalize(localPos));
	vec3 color = texture(material.texture_diffuse1, uv).rgb;
	FragColor = vec4(color, 1.0);
}