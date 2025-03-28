﻿#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左
  //警告：行列変換の結合順序は右結合（Right-to-Left Composition）で実施要
}