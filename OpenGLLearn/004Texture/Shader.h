// debug经验
// Shader构造失败时，只打印信息是不够的 --> 根据着色器程序ID的值来判断 ，ID是0表示无效
// 封装的set函数设置颜色失败 --> API用错了
// デバッグ経験
// シェーダー構築失敗時の処理不備 → シェーダープログラムIDの値(0)で有効性判定が必要
// カプセル化したset関数が色を設置失敗 → APIの誤用


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
	//程序ID 
	//プログラムID
	GLuint ID = 0; 

	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	//使用/激活程序
	// プログラムの使用/有効化
	bool Use();

	//删除Shader程序
	// シェーダープログラムを削除する
	void Remove();

	//uniform工具函数
	// uniformユーティリティ関数
	void SetBool(const string &name, bool value) const;
	void SetInt(const string& name, int value) const;
	void SetFloat(const string& name, float value) const;
	void Set4F(const string& name, float value0, float value1, float value2, float value3) const;

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

