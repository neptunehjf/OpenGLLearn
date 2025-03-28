#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;
uniform vec3 uni_lightPos;
uniform vec3 uni_objectColor;
uniform vec3 uni_lightColor;
uniform float uni_ambientStrength;
uniform float uni_diffuseStrength;
uniform float uni_specularStrength;
uniform int uni_specularFactor;

out vec4 fragColor;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);

  vec3 fragPos = vec3(uni_view * vec4(aPos, 1.0));

  // transpose(inverse(uni_view) 此操作在GPU开销大，实际开发时尽可能放到CPU处理
  // transpose(inverse(uni_view)) この処理はGPU負荷が高いため実開発ではCPU側で計算推奨 
  vec3 normal = mat3(transpose(inverse(uni_view))) * aNormal; 
  vec3 lightPos = vec3(uni_view * vec4(uni_lightPos, 1.0));

  // 只有在法线的矩阵可以在计算前用mat3把w齐次向量截掉，因为法线只有方向特性，没有位置特性。但是这里光源位置也截掉就错了
  // 法線の行列計算時のみmat3でw成分を除去可能（法線は方向特性のみを持ち、位置特性がない）  
  // ただし光源位置座標で同様の処理を行うと座標計算が破綻する  
  //lightPos = mat3(uni_view) * uni_lightPos;  

  // 环境光照ambient
  // 環境光
  vec3 ambient = uni_ambientStrength * uni_lightColor;
	
  // 漫反射光照diffuse
  // 拡散反射光 
  vec3 norm = normalize(normal);
  vec3 lightDir = normalize(lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = uni_diffuseStrength * diff * uni_lightColor;
	
  // 镜面光照specular
  // 鏡面反射光
  vec3 viewDir = normalize(vec3(0.0, 0.0, 0.0) - fragPos);
  vec3 reflectDir = normalize(reflect(-lightDir, norm));
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), uni_specularFactor);
  vec3 specular = uni_specularStrength * spec * uni_lightColor;

  // 光照颜色与物体颜色混合，营造光照效果
  // 光の色と物体の色をブレンド
  fragColor = vec4((ambient + diffuse + specular) * uni_objectColor, 1.0);

}