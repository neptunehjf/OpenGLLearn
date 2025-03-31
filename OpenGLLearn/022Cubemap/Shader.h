#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "common.h"

#define LOG_LENGTH 512

class Shader
{
public:
	//程序ID 
	//プログラムID
	GLuint ID = 0; 

	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	//使用/激活程序
	// プログラムの使用/有効化
	bool Use() const;

	//删除Shader程序
	// シェーダープログラムを削除する
	void Remove();

	//uniform工具函数
	// uniformユーティリティ関数
	void SetBool(const string &name, bool value) const;
	void SetInt(const string& name, int value) const;
	void SetFloat(const string& name, float value) const;
	void Set4F(const string& name, float value0, float value1, float value2, float value3) const;
	void SetMat4(const string& name, mat4 matrix) const;
	void SetVec3(const string& name, vec3 vector) const;
	void SetVec4(const string& name, vec4 vector) const;
};

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	// 1.从硬盘读取shader源码
	// 1. ハードディスクからシェーダーソースコードを読み込む
	string vertexCode;
	string fragmentCode;
	ifstream vShaderFile;
	ifstream fShaderFile;
	// 设置抛出异常类型
	// スロー例外型の指定
	vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		// 打开文件
		// ファイルを開く
		vShaderFile.open(vertexShaderPath);
		fShaderFile.open(fragmentShaderPath);
		// 读取到数据流
		// データストリームに読み込む
		stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// 关闭文件流
		// ファイルストリームを閉じる
		vShaderFile.close();
		fShaderFile.close();
		// 从数据流取出str
		// データストリームから文字列を取得
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(ifstream::failure e)
	{
		cout << "Read shader file: exception occur!" << endl;
		cerr << "Error reading file: " << e.what() << endl;
		cerr << "Error code: " << e.code() << endl;
	}

	// 获取shader源码
	// シェーダーソースコードを取得
	const char* vertexShaderSource = vertexCode.c_str();
	const char* fragmentShaderSource = fragmentCode.c_str();

	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	// 编译shader代码
	// シェーダーコードをコンパイル
	GLuint vertexShader = 0;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);  // 在这里直接用 &vertexCode.c_str()报错， 因为vertexCode.c_str()不是左值。
	// vertexCode.c_str()は左辺値ではないため、直接&演算子を適用するとコンパイルエラーが発生します

	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
		cout << "Vertex shader compile failed!\n" << infoLog << endl; 
	}

	GLuint fragmentShader = 0;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
		cout << "Fragment shader compile failed!\n" << infoLog << endl;
	}

	// 链接着色器程序
	// シェーダープログラムをリンク
	ID = glCreateProgram();
	if (ID == 0)
	{
		cout << "Shader program create failed!\n" << endl;
	}

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(ID, sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed!\n" << infoLog << endl;

		// 连接失败，shader程序应该置为不可用
		// リンク失敗時、シェーダープログラムを無効状態に設定
		ID = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

bool Shader::Use() const
{
	if (ID == 0)
		return false;
	else
		glUseProgram(ID);  // 开销相对较小，至少在我这个项目，重复调用问题也不大
		// オーバーヘッドが比較的小さく、少なくとも当プロジェクトでは多重呼び出しでも問題が発生しない

	return true;
}

void Shader::Remove()
{
	glDeleteProgram(ID);
}

void Shader::SetBool(const string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (GLint)value);
}

void Shader::SetInt(const string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::Set4F(const string& name, float value0, float value1, float value2, float value3) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value0, value1, value2, value3);  
}

void Shader::SetMat4(const string& name, mat4 matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
}

void Shader::SetVec3(const string& name, vec3 vector) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value_ptr(vector));
}

void Shader::SetVec4(const string& name, vec4 vector) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, value_ptr(vector));
}


