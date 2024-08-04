#pragma once

#include "Mesh.h"
#include "namespace.h"

const vector<Vertex> g_planeVertices = {
     // positions                 // normal                  // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	{vec3(5.0f, -0.5f,  5.0f),    vec3(0.0f,  1.0f,  0.0f),  vec2(2.0f, 0.0f)},
	{vec3(-5.0f, -0.5f,  5.0f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 0.0f)},
	{vec3(-5.0f, -0.5f, -5.0f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 2.0f)},
	{vec3(5.0f, -0.5f, -5.0f),    vec3(0.0f,  1.0f,  0.0f),  vec2(2.0f, 2.0f)}      // 纹理坐标越大，贴图越小越精细，但是贴图感更强
};

const vector<Vertex> g_cubeVertices = 
{
	//位置                         //法线                     //纹理坐标
	{vec3(-0.5f, -0.5f, -0.5f),   vec3(0.0f,  0.0f, -1.0f),  vec2(0.0f, 0.0f)},
	{vec3(0.5f, -0.5f, -0.5f),    vec3(0.0f,  0.0f, -1.0f),  vec2(1.0f, 0.0f)},
	{vec3(0.5f,  0.5f, -0.5f),    vec3(0.0f,  0.0f, -1.0f),  vec2(1.0f, 1.0f)},
	{vec3(-0.5f,  0.5f, -0.5f),   vec3(0.0f,  0.0f, -1.0f),  vec2(0.0f, 1.0f)},

	{vec3(-0.5f, -0.5f,  0.5f),   vec3(0.0f,  0.0f, 1.0f),   vec2(0.0f, 0.0f)},
	{vec3(0.5f, -0.5f,  0.5f),    vec3(0.0f,  0.0f, 1.0f),   vec2(1.0f, 0.0f)},
	{vec3(0.5f,  0.5f,  0.5f),    vec3(0.0f,  0.0f, 1.0f),   vec2(1.0f, 1.0f)},
	{vec3(-0.5f,  0.5f,  0.5f),   vec3(0.0f,  0.0f, 1.0f),   vec2(0.0f, 1.0f)},

	{vec3(-0.5f,  0.5f,  0.5f),   vec3(-1.0f,  0.0f, 0.0f),  vec2(1.0f, 0.0f)},
	{vec3(-0.5f,  0.5f, -0.5f),   vec3(-1.0f,  0.0f, 0.0f),  vec2(1.0f, 1.0f)},
	{vec3(-0.5f, -0.5f, -0.5f),   vec3(-1.0f,  0.0f, 0.0f),  vec2(0.0f, 1.0f)},
	{vec3(-0.5f, -0.5f,  0.5f),   vec3(-1.0f,  0.0f, 0.0f),  vec2(0.0f, 0.0f)},

	{vec3(0.5f,  0.5f,  0.5f),    vec3(1.0f,  0.0f, 0.0f),   vec2(1.0f, 0.0f)},
	{vec3(0.5f,  0.5f, -0.5f),    vec3(1.0f,  0.0f, 0.0f),   vec2(1.0f, 1.0f)},
	{vec3(0.5f, -0.5f, -0.5f),    vec3(1.0f,  0.0f, 0.0f),   vec2(0.0f, 1.0f)},
	{vec3(0.5f, -0.5f,  0.5f),    vec3(1.0f,  0.0f, 0.0f),   vec2(0.0f, 0.0f)},

	{vec3(-0.5f, -0.5f, -0.5f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.0f, 1.0f)},
	{vec3(0.5f, -0.5f, -0.5f),    vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 1.0f)},
	{vec3(0.5f, -0.5f,  0.5f),    vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 0.0f)},
	{vec3(-0.5f, -0.5f,  0.5f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.0f, 0.0f)},

	{vec3(-0.5f,  0.5f, -0.5f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 1.0f)},
	{vec3(0.5f,  0.5f, -0.5f),    vec3(0.0f,  1.0f,  0.0f),  vec2(1.0f, 1.0f)},
	{vec3(0.5f,  0.5f,  0.5f),    vec3(0.0f,  1.0f,  0.0f),  vec2(1.0f, 0.0f)},
	{vec3(-0.5f,  0.5f,  0.5f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 0.0f)}
};

const vector<GLuint> g_indices =
{
    0, 1, 2,
    0, 2, 3,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 13, 14,
    12, 14, 15,
    16, 17, 18,
    16, 18, 19,
    20, 21, 22,
    20, 22, 23
};