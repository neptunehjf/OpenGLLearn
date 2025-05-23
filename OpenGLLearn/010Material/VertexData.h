﻿#pragma once

#include "glm/glm.hpp"
#include "glm//gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


#define POSITION_SIZE 3
#define NORMAL_SIZE 3
#define STRIDE_SIZE (POSITION_SIZE + POSITION_SIZE)

using namespace glm;

float vertex[] = {
    //位置               //法線
    -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f
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