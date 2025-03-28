#version 330 core

layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec3 aColor;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;

//out vec3 color;
out vec2 texCoord;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左
  //color = aColor;
  texCoord = aTexCoord;
}