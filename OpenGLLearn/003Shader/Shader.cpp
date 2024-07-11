#include <glad/glad.h>
#include <glfw/glfw3.h>


#include <iostream>

#include "VertexData.h"
#include "Shader.h"

using namespace std;

#define WIDTH 800
#define HEIGHT 600


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

	// 创建Shader程序
	Shader myShader("shader.vs", "shader.fs"); // 默认vs的工作路径是在ProjectDir下，所以使用相对路径的话，应该把shader文件也放到ProjectDir

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
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, COLOR_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//当目标是GL_ELEMENT_ARRAY_BUFFER的时候，VAO会储存glBindBuffer的函数调用。这也意味着它也会储存解绑调用，所以确保你没有在解绑VAO之前解绑索引数组缓冲，否则它就没有这个EBO配置了
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// 运行shader
	if (myShader.Use() == false)
	{
		cout << "Shader program invalid!" << endl;
		return -1;
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		//输入
		processInput(window);

		//渲染
		//清空窗口
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 至少在这个项目里， shader程序在渲染循环前跑一次就够了，shader程序只是参数变了，shader程序本身没有变
		//if (myShader.Use() == false)
		//{
		//	cout << "Shader program invalid!" << endl;
		//	return -1;
		//}

		//随时间生成颜色
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		myShader.Set4F("ourColor", 0.0f, greenValue, 0.0f, 1.0f);

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
	myShader.Remove();

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