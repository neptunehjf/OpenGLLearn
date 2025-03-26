#pragma once

#define STRIDE_SIZE 6
#define POSITION_SIZE 3
#define COLOR_SIZE 3

float vertex[] =
{
	 //位置               //颜色
	 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // 0
	 0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // 1
	-0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f, // 2
	 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // 3
	-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, // 4
};

unsigned int index[] =
{
	0, 1, 2,
	0, 3, 4
};