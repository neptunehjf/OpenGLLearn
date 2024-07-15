#pragma once

#define STRIDE_SIZE 8
#define POSITION_SIZE 3
#define COLOR_SIZE 3
#define TEXCOORD_SIZE 2

float vertex[] =
{
	 //λ��               //��ɫ             //��ͼ����
	 0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.6f, 0.7f, //����
	 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  0.6f, 0.6f, //����
	 -0.5f, 0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 0.7f, //����
	-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.5f, 0.6f, //����
};

unsigned int index[] =
{
	0, 1, 2,
	1, 2, 3
};