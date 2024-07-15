// Shader����ʧ��ʱ��ֻ��ӡ��Ϣ�ǲ����� --> ������ɫ������ID��ֵ���ж� ��ID��0��ʾ��Ч
// ��װ��set����������ɫʧ�� --> API�ô���

#ifndef SHADER_H  
#define SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


using namespace std;

#define LOG_LENGTH 512

class Shader
{
public:
	//����ID 
	GLuint ID = 0; 

	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	//ʹ��/�������
	bool Use();

	//ɾ��Shader����
	void Remove();

	//uniform���ߺ���

	void SetBool(const string &name, bool value) const;
	void SetInt(const string& name, int value) const;
	void SetFloat(const string& name, float value) const;
	void Set4F(const string& name, float value0, float value1, float value2, float value3) const;

};

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	// 1.��Ӳ�̶�ȡshaderԴ��
	string vertexCode;
	string fragmentCode;
	ifstream vShaderFile;
	ifstream fShaderFile;
	// �����׳��쳣����
	vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		// ���ļ�
		vShaderFile.open(vertexShaderPath);
		fShaderFile.open(fragmentShaderPath);
		// ��ȡ��������
		stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// �ر��ļ���
		vShaderFile.close();
		fShaderFile.close();
		// ��������ȡ��str
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(ifstream::failure e)
	{
		cout << "Read shader file: exception occur!" << endl;
		cerr << "Error reading file: " << e.what() << endl;
		cerr << "Error code: " << e.code() << endl;
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

	// ������ɫ������
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

		// ����ʧ�ܣ�shader����Ӧ����Ϊ������
		ID = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

bool Shader::Use()
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


#endif // !SHADER_H

