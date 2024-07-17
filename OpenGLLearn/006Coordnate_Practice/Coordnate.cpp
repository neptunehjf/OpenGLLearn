#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>

#include "VertexData.h"
#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "glm/glm.hpp"
#include "glm//gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace glm;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int main()
{
	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	// ��ʼ��
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ���ƴ���
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenglWindow", NULL, NULL);

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

	// ����Shader����
	Shader myShader("shader.vs", "shader.fs"); // Ĭ��vs�Ĺ���·������ProjectDir�£�����ʹ�����·���Ļ���Ӧ�ð�shader�ļ�Ҳ�ŵ�ProjectDir

	// ���Դ�VAO������ shader�Ķ�������
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // VBO glVertexAttribPointer ������VAO������д

	// �洢�������ݵ��Դ�VBO
	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// �洢�±����ݵ��Դ�EBO
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	/* �洢��ͼ���ݵ��Դ� */
	// ��תy�ᣬʹͼƬ��opengl����һ��
	stbi_set_flip_vertically_on_load(true);
	// �����Դ�ռ䲢��GL_TEXTURE_2D����
	GLuint texture0 = 0;
	glGenTextures(1, &texture0);
	glBindTexture(GL_TEXTURE_2D, texture0); // �󶨲���Ҫô�Ƕ�Ҫô��д��������Ҫд
	// ����GL_TEXTURE_2D�Ļ��ƣ����˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ������ͼ��ת��Ϊ��������
	int width, height, channel;
	unsigned char *data = stbi_load("stone_wall.jpg", &width, &height, &channel, 0);
	if (data)
	{
		// �Դ�������ͼ����
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
	{
		cout << "Failed to load texture0��" << endl;
		return -1;
	}
	// ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
	stbi_image_free(data);

	// �����Դ�ռ䲢��GL_TEXTURE_2D����
	GLuint texture1 = 0;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1); // �󶨲���Ҫô�Ƕ�Ҫô��д��������Ҫд
	// ����GL_TEXTURE_2D�Ļ��ƣ����˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ������ͼ��ת��Ϊ��������
	data = stbi_load("awesomeface.png", &width, &height, &channel, 0);
	if (data)
	{
		// �Դ�������ͼ����
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture1��" << endl;
		return -1;
	}
	// ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
	stbi_image_free(data);

	// ���嶥�����ԵĽ�����ʽ
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, COLOR_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// ���
	glBindVertexArray(0);// �ر�VAO������

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//��Ŀ����GL_ELEMENT_ARRAY_BUFFER��ʱ��VAO�ᴢ��glBindBuffer�ĺ������á���Ҳ��ζ����Ҳ�ᴢ������ã�����ȷ����û���ڽ��VAO֮ǰ����������黺�壬��������û�����EBO������
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// ����shader
	if (myShader.Use() == false)
	{
		cout << "Shader program invalid!" << endl;
		return -1;
	}

	// ����sampler������Ӧ����Ԫ ��uniform�Ĳ���������shader����֮��
	myShader.SetInt("uni_texture0", 0);
	myShader.SetInt("uni_texture1", 1);

	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);

	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))
	{
		//����
		processInput(window);

		// ���buffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		// ����
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0); // ������󶨻���������sampler2D ����Ԫ0��

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1); // ������󶨻���������sampler2D ����Ԫ1��

		glBindVertexArray(VAO); // draw������VAO�����Ķ�    �ɴ���VBO EBO attrpoint�İ󶨲������������

		/* ���ɱ任���� */
		// view����
		mat4 view;
		view = translate(view, vec3(1.0f, 2.0f, -3.0f)); // ����������ǰ(-z)�ƶ����൱����������(+z)�ƶ���
		myShader.SetMat4("uni_view", view);
		// ͶӰ����
		mat4 projection;
		projection = perspective(radians(75.0f), (float)((WINDOW_WIDTH / WINDOW_HEIGHT) - 0.6), 0.1f, 100.0f);
		myShader.SetMat4("uni_projection", projection);

		for (int i = 0; i < 10; i++)
		{
			// model����
			mat4 model;
			model = translate(model, cubePositions[i]);
			if (i % 3 == 0)
			{
				model = rotate(model, (i + 1) * radians(50.0f) * (float)glfwGetTime(), vec3(0.5f, 1.0f, 0.0f));
			}
			myShader.SetMat4("uni_model", model);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// ���������� ��ѯ�¼�
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &texture0);
	glDeleteTextures(1, &texture1);
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