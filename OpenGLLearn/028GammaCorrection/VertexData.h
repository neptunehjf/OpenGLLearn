#pragma once

#include "Mesh.h"
#include "common.h"

const vector<Vertex> g_planeVertices = {
     // positions                 // normal                  // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	{vec3(5.0f, 1.0f,  5.0f),    vec3(0.0f,  1.0f,  0.0f),  vec2(5.0f, 0.0f)},
	{vec3(-5.0f, 1.0f,  5.0f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 0.0f)},
	{vec3(-5.0f, 1.0f, -5.0f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 5.0f)},
	{vec3(5.0f, 1.0f, -5.0f),    vec3(0.0f,  1.0f,  0.0f),  vec2(5.0f, 5.0f)}      // ��������Խ����ͼԽСԽ��ϸ��������ͼ�и�ǿ
};
const vector<GLuint> g_planeIndices =
{
    0, 1, 2,
    0, 2, 3
};

// Ĭ�������ڷ��߷���������ʱ�������£������档
const vector<Vertex> g_cubeVertices = 
{
	//λ��                         //����                     //��������
    // Back face
    {vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 0.0f, -1.0f),  vec2(0.0f, 0.0f)}, // Bottom-right
    {vec3(0.5f,  0.5f, -0.5f),  vec3(0.0f, 0.0f, -1.0f),  vec2(1.0f, 1.0f)},  // top-left
    {vec3(0.5f, -0.5f, -0.5f),  vec3(0.0f, 0.0f, -1.0f),  vec2(1.0f, 0.0f)},  // bottom-left         
    {vec3(-0.5f,  0.5f, -0.5f),  vec3(0.0f, 0.0f, -1.0f),  vec2(0.0f, 1.0f)}, // top-right
    // Front face
    {vec3(-0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f,  1.0f),  vec2(0.0f, 0.0f)}, // bottom-left
    {vec3(0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f,  1.0f),  vec2(1.0f, 0.0f)}, // bottom-right
    {vec3(0.5f,  0.5f,  0.5f),  vec3(0.0f, 0.0f,  1.0f),  vec2(1.0f, 1.0f)}, // top-right
    {vec3(-0.5f,  0.5f,  0.5f),  vec3(0.0f, 0.0f,  1.0f),  vec2(0.0f, 1.0f)}, // top-left
    // Left face
    {vec3(-0.5f,  0.5f,  0.5f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(1.0f, 0.0f)}, // top-right
    {vec3(-0.5f,  0.5f, -0.5f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(1.0f, 1.0f)}, // top-left
    {vec3(-0.5f, -0.5f, -0.5f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(0.0f, 1.0f)}, // bottom-left
    {vec3(-0.5f, -0.5f,  0.5f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(0.0f, 0.0f)}, // bottom-right
    // Right face
    {vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f,  0.0f, 0.0f),  vec2(1.0f, 0.0f)}, // top-left
    {vec3(0.5f, -0.5f, -0.5f),  vec3(1.0f,  0.0f, 0.0f),  vec2(0.0f, 1.0f)}, // bottom-right
    {vec3(0.5f,  0.5f, -0.5f),  vec3(1.0f,  0.0f, 0.0f),  vec2(1.0f, 1.0f)}, // top-right         
    {vec3(0.5f, -0.5f,  0.5f),  vec3(1.0f,  0.0f, 0.0f),  vec2(0.0f, 0.0f)}, // bottom-left     
    // Bottom face
    {vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, -1.0f, 0.0f),  vec2(0.0f, 1.0f)}, // top-right
    {vec3(0.5f, -0.5f, -0.5f),  vec3(0.0f, -1.0f, 0.0f),  vec2(1.0f, 1.0f)}, // top-left
    {vec3(0.5f, -0.5f,  0.5f),  vec3(0.0f, -1.0f, 0.0f),  vec2(1.0f, 0.0f)}, // bottom-left
    {vec3(-0.5f, -0.5f,  0.5f),  vec3(0.0f, -1.0f, 0.0f),  vec2(0.0f, 0.0f)}, // bottom-right
    // Top face
    {vec3(-0.5f,  0.5f, -0.5f),  vec3(0.0f,  1.0f, 0.0f),  vec2(0.0f, 1.0f)}, // top-left
    {vec3(0.5f,  0.5f,  0.5f),  vec3(0.0f,  1.0f, 0.0f),  vec2(1.0f, 0.0f)}, // bottom-right
    {vec3(0.5f,  0.5f, -0.5f),  vec3(0.0f,  1.0f, 0.0f),  vec2(1.0f, 1.0f)}, // top-right     
    {vec3(-0.5f,  0.5f,  0.5f),  vec3(0.0f,  1.0f, 0.0f),  vec2(0.0f, 0.0f)}  // bottom-left  
};
const vector<GLuint> g_cubeIndices =
{
    0, 1, 2,
    0, 3, 1,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 13, 14,
    12, 15, 13,
    16, 17, 18,
    16, 18, 19,
    20, 21, 22,
    20, 23, 21
};

const vector<Vertex> g_squareVertices = {
   // positions                  // normal                  // texture Coords
   {vec3(0.0f, 0.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 0.0f)},
   {vec3(1.0f, 0.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 0.0f)},
   {vec3(1.0f, 1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 1.0f)},
   {vec3(0.0f, 1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)}
};
const vector<GLuint> g_squareIndices =
{
	0, 1, 2,
	0, 2, 3
};


// ����normalû�����壬����Ϊ�˸���֮ǰ�Ĵ��룬����normal
const vector<Vertex> g_screenVertices = {
    // positions                    // normal                  // texture Coords
    {vec3(-1.0f,  1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 1.0f)},
    {vec3(-1.0f, -1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 0.0f)},
    {vec3( 1.0f, -1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 0.0f)},
    {vec3( 1.0f,  1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)}
};

const vector<GLuint> g_screenIndices =
{
    0, 1, 2,
    0, 2, 3
};

// ����normalû�����壬����Ϊ�˸���֮ǰ�Ĵ��룬����normal
const vector<Vertex> g_mirrorVertices = {
    // positions                    // normal(dummy)           // texture Coords(dummy) 
    {vec3(-0.2f,  1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 1.0f)},
    {vec3(-0.2f,  0.8f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 0.0f)},
    {vec3( 0.2f,  0.8f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 0.0f)},
    {vec3( 0.2f,  1.0f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)}
};

const vector<GLuint> g_mirrorIndices =
{
    0, 1, 2,
    0, 2, 3
};


const vector<Vertex> g_skyboxVertices =
{
    // positions                 // normal(dummy)          // texture Coords(dummy) 
    {vec3(-1.0f, -1.0f, -1.0f),  vec3(0.0f, 0.0f, -1.0f),  vec2(0.0f, 0.0f)},
    {vec3(1.0f,  1.0f, -1.0f),   vec3(0.0f, 0.0f, -1.0f),  vec2(1.0f, 1.0f)},
    {vec3(1.0f, -1.0f, -1.0f),   vec3(0.0f, 0.0f, -1.0f),  vec2(1.0f, 0.0f)},
    {vec3(-1.0f,  1.0f, -1.0f),  vec3(0.0f, 0.0f, -1.0f),  vec2(0.0f, 1.0f)},

    {vec3(-1.0f, -1.0f,  1.0f),  vec3(0.0f, 0.0f,  1.0f),  vec2(0.0f, 0.0f)},
    {vec3(1.0f, -1.0f,  1.0f),   vec3(0.0f, 0.0f,  1.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(1.0f,  1.0f,  1.0f),   vec3(0.0f, 0.0f,  1.0f),  vec2(1.0f, 1.0f)}, 
    {vec3(-1.0f,  1.0f,  1.0f),  vec3(0.0f, 0.0f,  1.0f),  vec2(0.0f, 1.0f)},

    {vec3(-1.0f,  1.0f,  1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(1.0f, 0.0f)},
    {vec3(-1.0f,  1.0f, -1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(1.0f, 1.0f)},
    {vec3(-1.0f, -1.0f, -1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(0.0f, 1.0f)},
    {vec3(-1.0f, -1.0f,  1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(0.0f, 0.0f)},

    {vec3(1.0f,  1.0f,  1.0f),   vec3(1.0f,  0.0f, 0.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(1.0f, -1.0f, -1.0f),   vec3(1.0f,  0.0f, 0.0f),  vec2(0.0f, 1.0f)}, 
    {vec3(1.0f,  1.0f, -1.0f),   vec3(1.0f,  0.0f, 0.0f),  vec2(1.0f, 1.0f)},      
    {vec3(1.0f, -1.0f,  1.0f),   vec3(1.0f,  0.0f, 0.0f),  vec2(0.0f, 0.0f)},    

    {vec3(-1.0f, -1.0f, -1.0f),  vec3(0.0f, -1.0f, 0.0f),  vec2(0.0f, 1.0f)},
    {vec3(1.0f, -1.0f, -1.0f),   vec3(0.0f, -1.0f, 0.0f),  vec2(1.0f, 1.0f)}, 
    {vec3(1.0f, -1.0f,  1.0f),   vec3(0.0f, -1.0f, 0.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(-1.0f, -1.0f,  1.0f),  vec3(0.0f, -1.0f, 0.0f),  vec2(0.0f, 0.0f)},

    {vec3(-1.0f,  1.0f, -1.0f),  vec3(0.0f,  1.0f, 0.0f),  vec2(0.0f, 1.0f)},
    {vec3(1.0f,  1.0f,  1.0f),   vec3(0.0f,  1.0f, 0.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(1.0f,  1.0f, -1.0f),   vec3(0.0f,  1.0f, 0.0f),  vec2(1.0f, 1.0f)},  
    {vec3(-1.0f,  1.0f,  1.0f),  vec3(0.0f,  1.0f, 0.0f),  vec2(0.0f, 0.0f)}  
};
const vector<GLuint> g_skyboxIndices =
{
    0, 1, 2,
    0, 3, 1,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 13, 14,
    12, 15, 13,
    16, 17, 18,
    16, 18, 19,
    20, 21, 22,
    20, 23, 21
};

const vector<Vertex> g_particleVertices = {
    // positions                  // normal(dummy)           // texture Coords(dummy) 
    {vec3(0.3f, 0.5f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 1.0f)},
    {vec3(0.4f, 0.2f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 0.0f)},
    {vec3(0.3f, 0.3f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 0.0f)},
    {vec3(0.5f, 0.1f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
    {vec3(0.4f, 0.4f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
    {vec3(0.5f, 0.5f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
    {vec3(0.3f, 0.1f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
};

const vector<GLuint> g_particleIndices =
{
    0, 1, 2, 3, 4, 5, 6
};

const vector<Vertex> g_GMTestVertices = {
    // positions                   // color(��ʱ��normal��װcolor�����濴����û�и��õķ���)  // texture Coords(dummy)
    {vec3(-0.5f, 0.5f, 0.0f),      vec3(1.0f,  0.0f,  0.0f),                             vec2(0.0f, 1.0f)},
    {vec3(-0.5f, -0.5f, 0.0f),     vec3(0.0f,  1.0f,  0.0f),                             vec2(1.0f, 1.0f)},
    {vec3(0.5f, 0.5f, 0.0f),       vec3(0.0f,  0.0f,  1.0f),                             vec2(0.0f, 0.0f)},
    {vec3(0.5f, -0.5f, 0.0f),      vec3(1.0f,  1.0f,  0.0f),                             vec2(1.0f, 0.0f)},
};

const vector<GLuint> g_GMTestIndices =
{
    0, 1, 2, 3
};

const vector<float> g_InstanceTestV = {
    // λ��           // ��ɫ
  { -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
    -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,
     0.05f,  0.05f,  0.0f, 1.0f, 1.0f }
};

const vector<uint> g_InstanceTestI = {
   { 0, 1, 2,
     0, 1, 3 }
};

const vector<uint> g_InstanceTestP = {
    { 2, 3 }
};