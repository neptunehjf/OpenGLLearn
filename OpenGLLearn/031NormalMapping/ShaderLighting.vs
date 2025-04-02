#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitTangent;

uniform mat4 uni_model;
uniform mat4 dirLightSpace;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

out VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 position;
	mat4 view;
	mat4 projection;
	vec4 fragPosLightSpace;
	mat3 TBN;
} vs_out;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

  vs_out.fragPos = vec3(uni_model * vec4(aPos, 1.0));
  vs_out.normal = mat3(transpose(inverse(uni_model))) * aNormal;
  vs_out.texCoord = aTexCoord;
  vs_out.position = uni_model * vec4(aPos, 1.0);
  vs_out.view = uni_view;
  vs_out.projection = uni_projection;
  vs_out.fragPosLightSpace = dirLightSpace * vec4(vs_out.fragPos, 1.0);

  // 切线空间->Local空间->World空间 
  // 理论上，法线保留在切线空间，而其他光照参数转成切线空间，也可以正确计算光照。
  // 这样的话，一些比较固定的光照参数可以放到顶点着色器，然后转到切线空间
  // 因为顶点着色器的调用频次要远小于片段着色器，所以优化很明显
  // 逆矩阵开销大于置换矩阵运算。可以利用正交矩阵的置换矩阵与它的逆矩阵相等这一特性，用置换矩阵运算代替逆矩阵运算，可以减小开销

  // 接空間 -> ローカル空間 -> ワールド空間
  // 理論的には、法線を接空間に保持し、他の光照パラメータを接空間に変換すれば、光照計算を正しく行えます
  // この場合、比較的固定された光照パラメータを頂点シェーダーで接空間に変換することが可能です
  // 頂点シェーダーの呼び出し頻度はフラグメントシェーダーよりもはるかに少ないため、最適化が顕著です
  // 逆行列のコストは転置行列演算よりも大きい。直交行列の転置行列と逆行列が等しい特性を利用し、逆行列演算の代わりに転置行列演算を使用することで、コストを削減できます

  vec3 T = normalize(vec3(transpose(inverse(uni_model)) * vec4(aTangent,   0.0)));
  vec3 B = normalize(vec3(transpose(inverse(uni_model)) * vec4(aBitTangent, 0.0)));
  vec3 N = normalize(vec3(transpose(inverse(uni_model)) * vec4(aNormal,    0.0)));
  mat3 TBN = mat3(T, B, N);
  vs_out.TBN = TBN;
}