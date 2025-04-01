#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// 着色器属性最大支持vec4类型，而mat4包含4个vec4，相当于location3 4 5 6
// シェーダー属性はvec4型まで対応。mat4は4つのvec4で構成されるため、location3・4・5・6を占有
layout(location = 3) in mat4 aInstAsteriod;  


layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

uniform mat4 extra_model;

out VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} vs_out;

void main()
{
  gl_Position = uni_projection * uni_view * extra_model * aInstAsteriod * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

  vs_out.fragPos = vec3(extra_model * aInstAsteriod * vec4(aPos, 1.0));
  vs_out.normal = mat3(transpose(inverse(aInstAsteriod))) * aNormal;
  vs_out.texCoord = aTexCoord;
}