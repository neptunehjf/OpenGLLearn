#pragma once

#include "Mesh.h"

const std::vector<Vertex> g_vertices = 
{
	//位置                              //法线                          //纹理坐标
	{glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec2(0.0f, 0.0f)},
	{glm::vec3(0.5f, -0.5f, -0.5f),    glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec2(1.0f, 0.0f)},
	{glm::vec3(0.5f,  0.5f, -0.5f),    glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec2(1.0f, 1.0f)},
	{glm::vec3(-0.5f,  0.5f, -0.5f),   glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec2(0.0f, 1.0f)},

	{glm::vec3(-0.5f, -0.5f,  0.5f),   glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(0.0f, 0.0f)},
	{glm::vec3(0.5f, -0.5f,  0.5f),    glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(1.0f, 0.0f)},
	{glm::vec3(0.5f,  0.5f,  0.5f),    glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(1.0f, 1.0f)},
	{glm::vec3(-0.5f,  0.5f,  0.5f),   glm::vec3(0.0f,  0.0f, 1.0f),   glm::vec2(0.0f, 1.0f)},

	{glm::vec3(-0.5f,  0.5f,  0.5f),   glm::vec3(-1.0f,  0.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
	{glm::vec3(-0.5f,  0.5f, -0.5f),   glm::vec3(-1.0f,  0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
	{glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(-1.0f,  0.0f, 0.0f),  glm::vec2(0.0f, 1.0f)},
	{glm::vec3(-0.5f, -0.5f,  0.5f),   glm::vec3(-1.0f,  0.0f, 0.0f),  glm::vec2(0.0f, 0.0f)},

	{glm::vec3(0.5f,  0.5f,  0.5f),    glm::vec3(1.0f,  0.0f, 0.0f),   glm::vec2(1.0f, 0.0f)},
	{glm::vec3(0.5f,  0.5f, -0.5f),    glm::vec3(1.0f,  0.0f, 0.0f),   glm::vec2(1.0f, 1.0f)},
	{glm::vec3(0.5f, -0.5f, -0.5f),    glm::vec3(1.0f,  0.0f, 0.0f),   glm::vec2(0.0f, 1.0f)},
	{glm::vec3(0.5f, -0.5f,  0.5f),    glm::vec3(1.0f,  0.0f, 0.0f),   glm::vec2(0.0f, 0.0f)},

	{glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, -1.0f, 0.0f),   glm::vec2(0.0f, 1.0f)},
	{glm::vec3(0.5f, -0.5f, -0.5f),    glm::vec3(0.0f, -1.0f, 0.0f),   glm::vec2(1.0f, 1.0f)},
	{glm::vec3(0.5f, -0.5f,  0.5f),    glm::vec3(0.0f, -1.0f, 0.0f),   glm::vec2(1.0f, 0.0f)},
	{glm::vec3(-0.5f, -0.5f,  0.5f),   glm::vec3(0.0f, -1.0f, 0.0f),   glm::vec2(0.0f, 0.0f)},

	{glm::vec3(-0.5f,  0.5f, -0.5f),   glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
	{glm::vec3(0.5f,  0.5f, -0.5f),    glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 1.0f)},
	{glm::vec3(0.5f,  0.5f,  0.5f),    glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
	{glm::vec3(-0.5f,  0.5f,  0.5f),   glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 0.0f)}
};

const std::vector<GLuint> g_indices =
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