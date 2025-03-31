﻿#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord; // 無視してもいい、コード流用のため

uniform mat4 uni_model;
uniform mat4 uni_view;
uniform mat4 uni_projection;

out vec3 normal;
out vec3 position;

void main()
{
  gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);

  normal = mat3(transpose(inverse(uni_model))) * aNormal;
  position = vec3(uni_model * vec4(aPos, 1.0));
}