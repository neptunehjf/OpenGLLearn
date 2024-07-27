#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>
#include <cstdio>

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

glm::vec3 cubePosition[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool LoadTexture(const string&& filePath, GLuint& texture);

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

	/* ������ͼ */
    // ��תy�ᣬʹͼƬ��opengl����һ��
	stbi_set_flip_vertically_on_load(true);

	// ������ͼ
	GLuint texture_diffuse = 0;
	GLuint texture_specular = 0;
	bool ret1, ret2;
	ret1 = LoadTexture("container_diffuse.png", texture_diffuse);
	ret2 = LoadTexture("container_specular.png", texture_specular);
	if (ret1 == false || ret2 == false)
		return -1;

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
	glVertexAttribPointer(2, TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);

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
		//static float ligthShift = 0.0f;
		static vec3 light_ambient = vec3(0.2f);
		static vec3 light_diffuse = vec3(0.8f);
		static vec3 light_specular = vec3(1.0f);
		static float light_innerCos = 5.0f;
		static float light_outerCos = 8.0f;

		static int material_shininess = 32;

		ImGui::Begin("Test Parameter");

		//camera
		ImGui::Text("CameraX %f | CameraY %f | CameraZ %f | CameraPitch %f | CameraYaw %f | CameraFov %f",
			myCam.camPos.x, myCam.camPos.y, myCam.camPos.z, myCam.pitchValue, myCam.yawValue, myCam.fov);

		//light
		//ImGui::SliderFloat("light position", &ligthShift, 0.0f, 10.0f);
		ImGui::ColorEdit3("light ambient", (float*)&light_ambient);
		ImGui::ColorEdit3("light diffuse", (float*)&light_diffuse);
		ImGui::ColorEdit3("light specular", (float*)&light_specular);
		ImGui::SliderFloat("light innerCos", &light_innerCos, 0.0f, 90.0f);
		ImGui::SliderFloat("light outerCos", &light_outerCos, 0.0f, 90.0f);

		//float constant = 1.0f; // ͨ������1������
		//float linear = 0.09f;
		//float quadratic = 0.032f;
		//const char* itemArray[] = { "										50", 
		//							"										100", 
		//							"										200", 
		//							"										600" };
		//static int item = 0;
		//ImGui::Combo("Light Fade Distance", &item, itemArray, IM_ARRAYSIZE(itemArray));

		//switch (item)
		//{
		//	case 0:
		//	{
		//		linear = 0.09f;
		//		quadratic = 0.032f;
		//		break;
		//	}
		//	case 1:
		//	{
		//		linear = 0.045f;
		//		quadratic = 0.0075f;
		//		break;
		//	}
		//	case 2:
		//	{
		//		linear = 0.022f;
		//		quadratic = 0.0019f;
		//		break;
		//	}
		//	case 3:
		//	{
		//		linear = 0.007f;
		//		quadratic = 0.0002f;
		//		break;
		//	}
		//	default:
		//	{
		//		cout << "Light Fade Distance Error!" << endl;
		//		break;
		//	}
		//		
		//}

		//material
		ImGui::SliderInt("material shininess", &material_shininess, 0, 256);

		//other
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		//����myShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
		myShader.Use();

		// ��������Ԫ �κ�uniform���ò���һ��Ҫ�ŵ�����Ӧ��shader������֮��  --����ͬ��shader�л����У���һ��shader��ص���д�����ݻᶪʧ����
        //Ҳ����˵������shader1֮����������shader2��֮ǰ��shader1���õľ���Ч�ˣ��������ֻ�ܷŵ���Ⱦѭ������ܷ�ѭ������
		myShader.SetInt("material.diffuse", 0);
		myShader.SetInt("material.specular", 1);

		//���λ����Ҫʵʱ���µģ�����������shader1֮����������shader2��shader1�����û���Ч��
		myShader.SetVec3("uni_viewPos", myCam.camPos); 

		myShader.SetVec3("light.lightPos", myCam.camPos);
		myShader.SetVec3("light.direction", myCam.camFront);
		myShader.SetVec3("light.ambient", light_ambient);
		myShader.SetVec3("light.diffuse", light_diffuse);
		myShader.SetVec3("light.specular", light_specular);
		myShader.SetFloat("light.innerCos", cos(radians(light_innerCos)));
		myShader.SetFloat("light.outerCos", cos(radians(light_outerCos)));
		//myShader.SetFloat("light.constant", 1.0f);
		//myShader.SetFloat("light.linear", linear);
		//myShader.SetFloat("light.quadratic", quadratic);

		myShader.SetInt("material.shininess", material_shininess);

		glBindVertexArray(VAO); // draw������VAO�����Ķ�    �ɴ���VBO EBO attrpoint�İ󶨲������������

		// ���Դ�,�Ϳ��Խ��ж�д����������Ҫ����д�����Դ��ʱ��û�취ͬʱ��ͬһ��GL_TEXTURE_2D��ֻ��������Ԫ������
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_diffuse);  //Ƭ����ɫ�������GL_TEXTURE0��ȡtexture_diffuse����ͼ����
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_specular); //Ƭ����ɫ�������GL_TEXTURE1��ȡtexture_specular����ͼ����

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
		for (unsigned int i = 0; i < 10; i++)
		{
			mat4 model = mat4(1.0f); // mat4��ʼ�������ʾ���ó�ʼ��Ϊ��λ������Ϊ�°汾mat4 model������ȫ0����
			model = translate(model, cubePosition[i]);
			float angle = 20.0f * i;
			model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			myShader.SetMat4("uni_model", model);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// ���
		glBindVertexArray(0);

		//vec3 lightPos = vec3(5 * cos(ligthShift), 10.0f, 5 * sin(ligthShift));

		//myShader.SetVec3("light.direction", vec3(-1.0, -1.0, -1.0));
		// ��Դģ�ͣ�һ����ɫ�ķ�����
		// ����lampShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
		//lampShader.Use();

		//mat4 model = mat4(1.0f); // ��ʼ��Ϊ��λ�������
		//model = scale(model, vec3(0.5f));
		//model = translate(model, lightPos);
		//model = rotate(model, radians(45.0f), vec3(1.0f, 1.0f, 0.0f));

		//lampShader.SetMat4("uni_view", view);
		//lampShader.SetMat4("uni_projection", projection);
		//lampShader.SetMat4("uni_model", model);
		//lampShader.SetVec3("uni_lightColor", vec3(1.0f));

		//glBindVertexArray(lightVAO);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//// ���
		//glBindVertexArray(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ���������� ��ѯ�¼�
		glfwSwapBuffers(window);

	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &texture_diffuse);
	glDeleteTextures(1, &texture_specular);
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

bool LoadTexture(const string&& filePath, GLuint& texture)
{
	// �����Դ�ռ䲢��GL_TEXTURE_2D����
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // �󶨲���Ҫô�Ƕ�Ҫô��д��������Ҫд
	// ����GL_TEXTURE_2D�Ļ��ƣ����˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ������ͼ��ת��Ϊ��������
	int width, height, channel;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channel, 0);

	GLenum format = 0;
	if (channel == 1)
		format = GL_RED;
	else if (channel == 3)
		format = GL_RGB;
	else if (channel == 4)
		format = GL_RGBA;


	//printf("**********************************************************\n");
	//for (int i = 1; i <= width * height; i++)
	//{
	//	printf("%02x ", data[i]);
	//	if (i % width == 0)
	//		printf("\n");
	//}
	//printf("**********************************************************\n");

	if (data)
	{
		// ��ͼ���� �ڴ� -> �Դ�
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		// ���ɶ༶������ͼ
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture��" << endl;
		return false;
	}
	// ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
	stbi_image_free(data);

	return true;
}