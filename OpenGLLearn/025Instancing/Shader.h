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
	//����ID 
	GLuint ID = 0; 

	Shader() {};

	Shader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath);

	//ʹ��/�������
	bool Use() const;

	//ɾ��Shader����
	void Remove();

	//uniform���ߺ���
	void SetBool(const string &name, bool value) const;
	void SetInt(const string& name, int value) const;
	void SetFloat(const string& name, float value) const;
	void Set4F(const string& name, float value0, float value1, float value2, float value3) const;
	void SetMat4(const string& name, mat4 matrix) const;
	void SetVec3(const string& name, vec3 vector) const;
	void SetVec4(const string& name, vec4 vector) const;

private:
	GLuint CompileShader(const char* shaderPath, GLuint shaderType);
};

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath = NULL)
{
	// ������ɫ��
	GLuint vertexShader = CompileShader(vertexShaderPath, GL_VERTEX_SHADER);

	// ������ɫ��
	GLuint geometryShader = 0;
	if (geometryShaderPath)
	{
		geometryShader = CompileShader(geometryShaderPath, GL_GEOMETRY_SHADER);
	}

	// Ƭ����ɫ��
	GLuint fragmentShader = CompileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	// ������ɫ������
	ID = glCreateProgram();
	if (ID == 0)
	{
		cout << "Shader program create failed!\n" << endl;
	}
	glAttachShader(ID, vertexShader);
	if (geometryShaderPath)
		glAttachShader(ID, geometryShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(ID, sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed!\n" << infoLog << endl;

		//ID = 0; ���ﲻ����0����Ϊ�����Ͳ��ܰ�ID delete��
	}

	glDeleteShader(vertexShader);
	if (geometryShaderPath)
		glDeleteShader(geometryShader);
	glDeleteShader(fragmentShader);
}

bool Shader::Use() const
{
	if (ID == 0)
		return false;
	else
		glUseProgram(ID);  // ������Խ�С���������������Ŀ���ظ���������Ҳ����

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


GLuint Shader::CompileShader(const char* shaderPath, GLuint shaderType)
{
	// 1.��Ӳ�̶�ȡshaderԴ��
	string code;
	ifstream shaderFile;
	// �����׳��쳣����
	shaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		// ���ļ�
		shaderFile.open(shaderPath);
		// ��ȡ��������
		stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		// �ر��ļ���
		shaderFile.close();
		// ��������ȡ��str
		code = shaderStream.str();
	}
	catch (ifstream::failure e)
	{
		cout << "Read shader file: exception occur!" << endl;
		cerr << "Error reading file: " << e.what() << endl;
		cerr << "Error code: " << e.code() << endl;
	}

	// ��ȡshaderԴ��
	const char* shaderSource = code.c_str();

	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	// ����shader����
	GLuint shader = 0;
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);  // ������ֱ���� &code.c_str()���� ��Ϊcode.c_str()������ֵ��
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		cout << "Shader compile failed!\n" << infoLog << endl;
	}

	return shader;
}

