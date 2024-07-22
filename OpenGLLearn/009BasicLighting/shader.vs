#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;

out vec3 fragPos;
out vec3 normalModel;
out vec3 normalView;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

  fragPos = vec3(uni_model * vec4(aPos, 1.0));
  normalModel = mat3(transpose(inverse(uni_model))) * aNormal;
  normalView = mat3(transpose(inverse(uni_view))) * aNormal;
}