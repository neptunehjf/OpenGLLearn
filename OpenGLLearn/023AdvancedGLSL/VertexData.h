#pragma once

#include "Mesh.h"
#include "common.h"

const vector<Vertex> g_planeVertices = {
     // positions                 // normal                  // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	{vec3(5.0f, -0.5f,  5.0f),    vec3(0.0f,  1.0f,  0.0f),  vec2(2.0f, 0.0f)},
	{vec3(-5.0f, -0.5f,  5.0f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 0.0f)},
	{vec3(-5.0f, -0.5f, -5.0f),   vec3(0.0f,  1.0f,  0.0f),  vec2(0.0f, 2.0f)},
	{vec3(5.0f, -0.5f, -5.0f),    vec3(0.0f,  1.0f,  0.0f),  vec2(2.0f, 2.0f)}      // 纹理坐标越大，贴图越小越精细，但是贴图感更强
};
const vector<GLuint> g_planeIndices =
{
    0, 1, 2,
    0, 2, 3
};

// 右手定则。默认情况下，顶点在法线方向上是逆时针的情况下，是正面。
// 右手の法則。デフォルトでは法線方向に対して頂点が反時計回りに配置された面を表面と判定します。
const vector<Vertex> g_cubeVertices = 
{
	//位置                         //法線                     //UV座標 
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


// 这里normal没有意义，但是为了复用之前的代码，加了normal
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

// 这里normal没有意义，但是为了复用之前的代码，加了normal
// このコンテキストにおけるnormal変数は意味を持たないが、既存コードの再利用性を維持するために追加されています
const vector<Vertex> g_mirrorVertices = {
    // positions                    // normal                  // texture Coords
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
    {vec3(-1.0f, -1.0f, -1.0f),  vec3(0.0f, 0.0f, -1.0f),  vec2(0.0f, 0.0f)},
    {vec3(1.0f,  1.0f, -1.0f),  vec3(0.0f, 0.0f, -1.0f),  vec2(1.0f, 1.0f)},
    {vec3(1.0f, -1.0f, -1.0f),  vec3(0.0f, 0.0f, -1.0f),  vec2(1.0f, 0.0f)},
    {vec3(-1.0f,  1.0f, -1.0f),  vec3(0.0f, 0.0f, -1.0f),  vec2(0.0f, 1.0f)},

    {vec3(-1.0f, -1.0f,  1.0f),  vec3(0.0f, 0.0f,  1.0f),  vec2(0.0f, 0.0f)},
    {vec3(1.0f, -1.0f,  1.0f),  vec3(0.0f, 0.0f,  1.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(1.0f,  1.0f,  1.0f),  vec3(0.0f, 0.0f,  1.0f),  vec2(1.0f, 1.0f)}, 
    {vec3(-1.0f,  1.0f,  1.0f),  vec3(0.0f, 0.0f,  1.0f),  vec2(0.0f, 1.0f)},

    {vec3(-1.0f,  1.0f,  1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(1.0f, 0.0f)},
    {vec3(-1.0f,  1.0f, -1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(1.0f, 1.0f)},
    {vec3(-1.0f, -1.0f, -1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(0.0f, 1.0f)},
    {vec3(-1.0f, -1.0f,  1.0f),  vec3(-1.0f, 0.0f, 0.0f),  vec2(0.0f, 0.0f)},

    {vec3(1.0f,  1.0f,  1.0f),  vec3(1.0f,  0.0f, 0.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(1.0f, -1.0f, -1.0f),  vec3(1.0f,  0.0f, 0.0f),  vec2(0.0f, 1.0f)}, 
    {vec3(1.0f,  1.0f, -1.0f),  vec3(1.0f,  0.0f, 0.0f),  vec2(1.0f, 1.0f)},      
    {vec3(1.0f, -1.0f,  1.0f),  vec3(1.0f,  0.0f, 0.0f),  vec2(0.0f, 0.0f)},    

    {vec3(-1.0f, -1.0f, -1.0f),  vec3(0.0f, -1.0f, 0.0f),  vec2(0.0f, 1.0f)},
    {vec3(1.0f, -1.0f, -1.0f),  vec3(0.0f, -1.0f, 0.0f),  vec2(1.0f, 1.0f)}, 
    {vec3(1.0f, -1.0f,  1.0f),  vec3(0.0f, -1.0f, 0.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(-1.0f, -1.0f,  1.0f),  vec3(0.0f, -1.0f, 0.0f),  vec2(0.0f, 0.0f)},

    {vec3(-1.0f,  1.0f, -1.0f),  vec3(0.0f,  1.0f, 0.0f),  vec2(0.0f, 1.0f)},
    {vec3(1.0f,  1.0f,  1.0f),  vec3(0.0f,  1.0f, 0.0f),  vec2(1.0f, 0.0f)}, 
    {vec3(1.0f,  1.0f, -1.0f),  vec3(0.0f,  1.0f, 0.0f),  vec2(1.0f, 1.0f)},  
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
    // positions                    // normal                  // texture Coords
    {vec3(0.3f, 0.5f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 1.0f)},
    {vec3(0.4f, 0.2f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(0.0f, 0.0f)},
    {vec3(0.3f, 0.3f, 0.0f),     vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 0.0f)},
    {vec3(0.5f, 0.1f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
    {vec3(0.4f, 0.4f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
    {vec3(0.5f, 0.5f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
    {vec3(0.3f, 0.1f, 0.0f),      vec3(0.0f,  0.0f,  1.0f),  vec2(1.0f, 1.0f)},
};

const vector<GLuint> g_particleIndices =
{
    0, 1, 2, 3, 4, 5, 6
};