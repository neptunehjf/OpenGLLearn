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

#define LOG_LENGTH 512

class Shader
{
public:
	//����ID 
	GLuint ID = 0; 

	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	//ʹ��/�������
	bool Use() const;

	//ɾ��Shader����
	void Remove();

	//uniform���ߺ���
	void SetBool(const std::string &name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void Set4F(const std::string& name, float value0, float value1, float value2, float value3) const;
	void SetMat4(const std::string& name, glm::mat4 matrix) const;
	void SetVec3(const std::string& name, glm::vec3 vector) const;
};

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	// 1.��Ӳ�̶�ȡshaderԴ��
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// �����׳��쳣����
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// ���ļ�
		vShaderFile.open(vertexShaderPath);
		fShaderFile.open(fragmentShaderPath);
		// ��ȡ��������
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// �ر��ļ���
		vShaderFile.close();
		fShaderFile.close();
		// ��������ȡ��str
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(std::ifstream::failure e)
	{
		std::cout << "Read shader file: exception occur!" << std::endl;
		std::cerr << "Error reading file: " << e.what() << std::endl;
		std::cerr << "Error code: " << e.code() << std::endl;
	}

	// ��ȡshaderԴ��
	const char* vertexShaderSource = vertexCode.c_str();
	const char* fragmentShaderSource = fragmentCode.c_str();

	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	// ����shader����
	GLuint vertexShader = 0;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);  // ������ֱ���� &vertexCode.c_str()���� ��ΪvertexCode.c_str()������ֵ��
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
		std::cout << "Vertex shader compile failed!\n" << infoLog << std::endl; 
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
		std::cout << "Fragment shader compile failed!\n" << infoLog << std::endl;
	}

	// ������ɫ������
	ID = glCreateProgram();
	if (ID == 0)
	{
		std::cout << "Shader program create failed!\n" << std::endl;
	}

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(ID, sizeof(infoLog), NULL, infoLog);
		std::cout << "Shader program link failed!\n" << infoLog << std::endl;

		// ����ʧ�ܣ�shader����Ӧ����Ϊ������
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
		glUseProgram(ID);

	return true;
}

void Shader::Remove()
{
	glDeleteProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (GLint)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::Set4F(const std::string& name, float value0, float value1, float value2, float value3) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value0, value1, value2, value3);  
}

void Shader::SetMat4(const std::string& name, glm::mat4 matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
}

void Shader::SetVec3(const std::string& name, glm::vec3 vector) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value_ptr(vector));
}


