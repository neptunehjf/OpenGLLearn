﻿#pragma once

#define STRIDE_SIZE 8
#define POSITION_SIZE 3
#define COLOR_SIZE 3
#define TEXCOORD_SIZE 2

float vertex[] =
{
	 //位置               //色            　 //UV座標
	 0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, //右上
	 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, //右下
	 -0.5f, 0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, //左上
	-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, //左下
};

unsigned int index[] =
{
	0, 1, 2,
	1, 2, 3
};