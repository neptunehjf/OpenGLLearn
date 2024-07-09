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
	GLuint fragmentShader[2];
	GLuint shaderProgram[2];

	//顶点shader
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

	//片段shader1
	fragmentShader[0] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[0], 1, &fragmentShaderSource1, NULL);
	glCompileShader(fragmentShader[0]);

	glGetShaderiv(fragmentShader[0], GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(fragmentShader[0], sizeof(infoLog), NULL, infoLog);
		cout << "Fragment shader1 compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	//片段shader2
	fragmentShader[1] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[1], 1, &fragmentShaderSource2, NULL);
	glCompileShader(fragmentShader[1]);

	glGetShaderiv(fragmentShader[1], GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(fragmentShader[1], sizeof(infoLog), NULL, infoLog);
		cout << "Fragment shader2 compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 链接着色器程序1
	shaderProgram[0] = glCreateProgram();
	glAttachShader(shaderProgram[0], vertexShader);
	glAttachShader(shaderProgram[0], fragmentShader[0]);
	glLinkProgram(shaderProgram[0]);

	glGetProgramiv(shaderProgram[0], GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(shaderProgram[0], sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 链接着色器程序2
	shaderProgram[1] = glCreateProgram();
	glAttachShader(shaderProgram[1], vertexShader);
	glAttachShader(shaderProgram[1], fragmentShader[1]);
	glLinkProgram(shaderProgram[1]);

	glGetProgramiv(shaderProgram[1], GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(shaderProgram[1], sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(shaderProgram[0]);
	glDeleteShader(shaderProgram[1]);

	GLuint VAO[2];
	glGenVertexArrays(2, VAO);
	GLuint VBO[2];
	glGenBuffers(2, VBO);

	/************ 第一个三角形 ***********/
	// 用VAO来管理 shader的顶点属性
	glBindVertexArray(VAO[0]); // VBO glVertexAttribPointer 操作向VAO上下文写

	// 存储顶点数据到VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex1), vertex1, GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	glVertexAttribPointer(0, ATTR_LENGTH, GL_FLOAT, GL_FALSE, ATTR_LENGTH * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文 这里可以不解绑，因为下面很快就绑定另一个VAO了
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/************ 第一个三角形 ***********/

	/************ 第二个三角形 ***********/
    // 用VAO来管理 shader的顶点属性
	glBindVertexArray(VAO[1]); // VBO glVertexAttribPointer 操作向VAO上下文写

	// 存储顶点数据到VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex2), vertex2, GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	glVertexAttribPointer(0, ATTR_LENGTH, GL_FLOAT, GL_FALSE, ATTR_LENGTH * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/************ 第二个三角形 ***********/

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		//输入
		processInput(window);

		//渲染之前清空窗口
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 绘制三角形
		glUseProgram(shaderProgram[0]);
		glBindVertexArray(VAO[0]); // draw操作从VAO上下文读    可代替VBO EBO attrpoint的绑定操作，方便管理
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		glUseProgram(shaderProgram[1]);
		glBindVertexArray(VAO[1]); // draw操作从VAO上下文读    可代替VBO EBO attrpoint的绑定操作，方便管理
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		//缓冲区交换 轮询事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2, VBO);
	glDeleteProgram(shaderProgram[0]);
	glDeleteProgram(shaderProgram[1]);

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