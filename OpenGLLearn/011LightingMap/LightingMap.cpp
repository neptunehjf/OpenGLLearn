#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>

#include "VertexData.h"
#include "Shader.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "glm/glm.hpp"
#include "glm//gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;
using namespace glm;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool LoadTexure(const string&& filePath, GLuint& texture);

Camera myCam(vec3(2.0f, 7.0f, 6.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

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

	// �������
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialze GLAD." << endl;
		glfwTerminate();
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	// ����Shader����
	Shader myShader("shader.vs", "shader.fs");
	Shader lampShader("lampShader.vs", "lampShader.fs"); // �Ʊ���shader��һ����ɫ�ķ�����

	// ���Դ�VAO������ shader�Ķ�������
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // VBO glVertexAttribPointer ������VAO������д

	// �洢�������ݵ��Դ�VBO
	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// ���嶥�����ԵĽ�����ʽ
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(0 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(0);
	//3 ���Ǽ�sizeof(GL_FLOAT)�ˣ��Ų��˰��졣�����Ժ�0Ҳд��0 * sizeof(GL_FLOAT)����ʽ�ɡ��������󵼱�Ĵ���
	glVertexAttribPointer(1, NORMAL_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT))); 
	glEnableVertexAttribArray(1);

	// �洢�±����ݵ��Դ�EBO
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	// ���
	glBindVertexArray(0);// �ر�VAO������

	// ר��Ϊ��Դ������һ��VAO�������������
	GLuint lightVAO = 0;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO); // VBO glVertexAttribPointer ������VAO������д
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(0 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// ���
	glBindVertexArray(0);// �ر�VAO������

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//��Ŀ����GL_ELEMENT_ARRAY_BUFFER��ʱ��VAO�ᴢ��glBindBuffer�ĺ������á���Ҳ��ζ����Ҳ�ᴢ������ã�����ȷ����û���ڽ��VAO֮ǰ����������黺�壬��������û�����EBO������
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// ���myShader������Ч��
	if (myShader.Use() == false)
	{
		cout << "myShader program invalid!" << endl;
		return -1;
	}

	// ���lampShader������Ч��
	if (lampShader.Use() == false)
	{
		cout << "lampShader program invalid!" << endl;
		return -1;
	}

	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);

	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))
	{
		//����
		processInput(window);
		glfwPollEvents();

		// ���buffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//Imgui
		static float ligthShift = 0.0f;
		static vec3 light_ambient = vec3(1.0f);
		static vec3 light_diffuse = vec3(1.0f);
		static vec3 light_specular = vec3(1.0f);

		static vec3 material_ambient = vec3(0.0215f, 0.1745f, 0.0215f);
		static vec3 material_diffuse = vec3(0.07568f, 0.61424f, 0.07568f);
		static vec3 material_specular = vec3(0.633f, 0.727811f, 0.633f);
		static int material_shininess = 0.6 * 128;


		ImGui::Begin("Test Parameter");

		//camera
		ImGui::Text("CameraX %f | CameraY %f | CameraZ %f | CameraPitch %f | CameraYaw %f | CameraFov %f",
			myCam.camPos.x, myCam.camPos.y, myCam.camPos.z, myCam.pitchValue, myCam.yawValue, myCam.fov);

		//light
		ImGui::SliderFloat("light position", &ligthShift, 0.0f, 10.0f);
		ImGui::ColorEdit3("light ambient", (float*)&light_ambient);
		ImGui::ColorEdit3("light diffuse", (float*)&light_diffuse);
		ImGui::ColorEdit3("light specular", (float*)&light_specular);

		//material
		ImGui::ColorEdit3("material ambient", (float *)&material_ambient);
		ImGui::ColorEdit3("material diffuse", (float*)&material_diffuse);
		ImGui::ColorEdit3("material specular", (float*)&material_specular);
		ImGui::SliderInt("material shininess", &material_shininess, 0, 256);

		//other
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		//����myShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
		myShader.Use();

		myShader.SetVec3("uni_viewPos", myCam.camPos); //���λ����ʵʱ���µ�

		/* ���ɱ任���� */
		// view���� world -> view
		mat4 view;

		myCam.setCamView();
		view = myCam.getCamView();
		myShader.SetMat4("uni_view", view);

		// ͶӰ���� view -> clip
		mat4 projection;
		float fov = myCam.getCamFov();

		projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f); // ֮ǰд��(float)(WINDOW_WIDTH / WINDOW_HEIGHT)�ˣ����ȶ�ʧ�����½����1
		myShader.SetMat4("uni_projection", projection);

		// model���� local -> world
		// ����
		mat4 model = mat4(1.0f); // mat4��ʼ�������ʾ���ó�ʼ��Ϊ��λ������Ϊ�°汾mat4 model������ȫ0����
		model = scale(model, vec3(2.5f));
		model = translate(model, vec3(0.0f, 0.0f, -0.0f));
		model = rotate(model, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
		myShader.SetMat4("uni_model", model);

		myShader.SetVec3("light.ambient", light_ambient);
		myShader.SetVec3("light.diffuse", light_diffuse);
		myShader.SetVec3("light.specular", light_specular);

		myShader.SetVec3("material.ambient", material_ambient);
		myShader.SetVec3("material.diffuse", material_diffuse);
		myShader.SetVec3("material.specular", material_specular);
		myShader.SetInt("material.shininess", material_shininess);

		glBindVertexArray(VAO); // draw������VAO�����Ķ�    �ɴ���VBO EBO attrpoint�İ󶨲������������
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		// ���
		glBindVertexArray(0);

		vec3 lightPos = vec3(5 * cos(ligthShift), 10.0f, 5 * sin(ligthShift));
		myShader.SetVec3("light.lightPos", lightPos);
		// ��Դģ�ͣ�һ����ɫ�ķ�����
		// ����lampShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
		lampShader.Use();

		model = mat4(1.0f); // ��ʼ��Ϊ��λ�������
		model = scale(model, vec3(0.5f));
		model = translate(model, lightPos);
		model = rotate(model, radians(45.0f), vec3(1.0f, 1.0f, 0.0f));

		lampShader.SetMat4("uni_view", view);
		lampShader.SetMat4("uni_projection", projection);
		lampShader.SetMat4("uni_model", model);
		lampShader.SetVec3("uni_lightColor", vec3(1.0f));

		glBindVertexArray(lightVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// ���
		glBindVertexArray(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ���������� ��ѯ�¼�
		glfwSwapBuffers(window);

	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	myShader.Remove();
	lampShader.Remove();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	/* ���ƽ�� */
	myCam.currentFrame = glfwGetTime();
	myCam.deltaTime = myCam.currentFrame - myCam.lastFrame;
	myCam.lastFrame = myCam.currentFrame;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		myCam.camPos += normalize(myCam.camFront) * myCam.camSpeed * myCam.deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		myCam.camPos -= normalize(myCam.camFront) * myCam.camSpeed * myCam.deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		myCam.camPos -= normalize(cross(myCam.camFront, myCam.camUp)) * myCam.camSpeed * myCam.deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		myCam.camPos += normalize(cross(myCam.camFront, myCam.camUp)) * myCam.camSpeed * myCam.deltaTime;
	}
}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
	/* ����ӽ� */

	float offsetX = posX - myCam.lastX;
	float offsetY = myCam.lastY - posY;
	myCam.lastX = posX;
	myCam.lastY = posY;

	// ����Ҽ������Ͳ�������Ϊ���Ҫ������Imgui
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		return;
	}

	myCam.yawValue += offsetX;
	myCam.pitchValue += offsetY;
	float speed = 1.0f;
	myCam.yawValue *= speed;
	myCam.pitchValue *= speed;

	if (myCam.pitchValue > 89.0f)
		myCam.pitchValue = 89.0f;
	if (myCam.pitchValue < -89.0f)
		myCam.pitchValue = -89.0f;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	/* ��ͷ���� */
	if (myCam.fov >= 1.0f && myCam.fov <= 95.0f)
		myCam.fov -= yoffset;
	if (myCam.fov <= 1.0f)
		myCam.fov = 1.0f;
	if (myCam.fov >= 95.0f)
		myCam.fov = 95.0f;
}

// ��תy�ᣬʹͼƬ��opengl����һ��
stbi_set_flip_vertically_on_load(true);

bool LoadTexure(const string&& filePath, GLuint& texture)
{
	// �����Դ�ռ䲢��GL_TEXTURE_2D����
	glGenTextures(1, &texture); //texture�����õĻ���������ֱ����texture�����˰ɣ�
	glBindTexture(GL_TEXTURE_2D, texture); // �󶨲���Ҫô�Ƕ�Ҫô��д��������Ҫд
	// ����GL_TEXTURE_2D�Ļ��ƣ����˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ������ͼ��ת��Ϊ��������
	int width, height, channel;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channel, 0);
	if (data)
	{
		// �Դ�������ͼ����
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture0��" << endl;
		return false;
	}
	// ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
	stbi_image_free(data);

	return true;
}