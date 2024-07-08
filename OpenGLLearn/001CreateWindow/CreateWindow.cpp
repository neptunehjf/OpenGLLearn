//Q1���ʤ�����ƥ�����ɥ��򒈴󤷤��Ȥ��˰ז�����ʾ����롣
// 
//Q2��framebuffer_size_callback�Ϥ��ޤ��ۤ������ʤ��褦�Ǥ���
//A2��callback�v����glViewport��ʹ�ä��ƥ�����ɥ��������˺Ϥ碌�ƥ�������I��Υ��������{�����ޤ���
// 
//Q3��������ɥ��򒈴󤷤Ƥ���¤��ꥢ�륿������Ф��ʤ��褦�Ǥ��� 
// 
//Q4���ʤ�����򥯥ꥢ���ʤ���Фʤ�ޤ��󤫣�


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenglWindow", NULL, NULL);

	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialze GLAD." << endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//������󥰡���`��
	while (!glfwWindowShouldClose(window))
	{
		//����
		processInput(window);

		//�������
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//�Хåե����Q���ȡ����٥�ȥݩ`���
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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