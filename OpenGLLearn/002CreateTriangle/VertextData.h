#pragma once

#define ATTR_LENGTH 3

float vertex[] =
{
	 0.5f,  0.5f, 0.0f,  //右上
	 0.5f, -0.5f, 0.0f,  //右下
	 -0.5f, 0.5f, 0.0f,  //左上
	-0.5f, -0.5f, 0.0f,  //左下
};

unsigned int index[] =
{
	0, 1, 2,
	1, 2, 3
};