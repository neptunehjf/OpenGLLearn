#pragma once

#define STRIDE_SIZE 6
#define POSITION_SIZE 3
#define COLOR_SIZE 3

float vertex[] =
{
	 //λ��               //��ɫ
	 0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, //����
	 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, //����
	 -0.5f, 0.5f, 0.0f,  0.0f, 0.0f, 1.0f, //����
	-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, //����
};

unsigned int index[] =
{
	0, 1, 2,
	1, 2, 3
};