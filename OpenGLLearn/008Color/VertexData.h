#pragma once

#include "glm/glm.hpp"
#include "glm//gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STRIDE_SIZE 3
#define POSITION_SIZE 3

using namespace glm;

float vertex[] = {
    //位置               
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f
};

unsigned int index[] =
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

//vec3 cubePositions[] = {
//vec3(0.0f,  0.0f,  0.0f),
//vec3(2.0f,  5.0f, -15.0f),
//vec3(-1.5f, -2.2f, -2.5f),
//vec3(-3.8f, -2.0f, -12.3f),
//vec3(2.4f, -0.4f, -3.5f),
//vec3(-1.7f,  3.0f, -7.5f),
//vec3(1.3f, -2.0f, -2.5f),
//vec3(1.5f,  2.0f, -2.5f),
//vec3(1.5f,  0.2f, -1.5f),
//vec3(-1.3f,  1.0f, -1.5f)
//};