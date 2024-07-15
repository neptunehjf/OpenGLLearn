#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec4 vertexColor;
out vec4 vertexPosition;

uniform float uni_xOffet;
//uniform vec4 ourColor;

void main()
{
  //gl_Position = vec4(aPos, 1.0);
  gl_Position = vec4(aPos.x + uni_xOffet, -aPos.y, aPos.z, 1.0);
  vertexPosition = gl_Position;
  vertexColor = vec4(aColor, 1.0);
  //vertexColor = ourColor;
}