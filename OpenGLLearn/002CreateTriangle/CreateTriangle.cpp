// Q1为什么顶点数据是float型
// Q2绘制是基于窗口的，要先设置上下文到窗口才能调用gladLoadGLLoader



#include <glad/glad.h>
#include <glfw/glfw3.h>


#include <iostream>

#include "VertextData.h"
#include "Shader.h"

using namespace std;

#define WIDTH 800
#define HEIGHT 600
#define LOG_LENGTH 512

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int main()
{
	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	// 初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 绘制窗口
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenglWindow", NULL, NULL);

	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialze GLAD." << endl;
		glfwTerminate();
		return -1;
	}

	//glViewport(0, 0, 800, 600);

	// 编译shader代码
	GLuint vertexShader = 0;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
		cout << "Vertex shader compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
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
		cout << "Fragment shader compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 链接着色器程序
	GLuint shaderProgram = 0;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// 用VAO来管理 shader的顶点属性
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // VBO glVertexAttribPointer 操作向VAO上下文写

	// 存储顶点数据到VBO
	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// 存储下标数据到EBO
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	
	// VBO数据关联到shader的顶点属性
	glVertexAttribPointer(0, ATTR_LENGTH, GL_FLOAT, GL_FALSE, ATTR_LENGTH * sizeof(GL_FLOAT), (void *)0);
	glEnableVertexAttribArray(0);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//当目标是GL_ELEMENT_ARRAY_BUFFER的时候，VAO会储存glBindBuffer的函数调用。这也意味着它也会储存解绑调用，所以确保你没有在解绑VAO之前解绑索引数组缓冲，否则它就没有这个EBO配置了
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	// 运行shader
	glUseProgram(shaderProgram);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		//输入
		processInput(window);

		//渲染之前清空窗口
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 绘制三角形
		glBindVertexArray(VAO); // draw操作从VAO上下文读    可代替VBO EBO attrpoint的绑定操作，方便管理
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//缓冲区交换 轮询事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}