#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;
uniform vec3 uni_lightPos;

out vec3 fragPos;
out vec3 normal;
out vec3 lightPos;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

  fragPos = vec3(uni_view * vec4(aPos, 1.0));
  normal = mat3(transpose(inverse(uni_view))) * aNormal; //transpose(inverse(uni_view) 此操作在GPU开销大，实际开发时尽可能放到CPU处理
  lightPos = vec3(uni_view * vec4(uni_lightPos, 1.0));
  //lightPos = mat3(uni_view) * uni_lightPos;  // 只有在法线矩阵可以在计算前用mat4把w齐次向量截掉，因为法线只有方向特性，没有位置特性。但是这里光源位置也截掉就错了
}