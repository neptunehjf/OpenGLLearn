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

#include <string>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool LoadTexture(const std::string&& filePath, GLuint& texture);
void loadModel(std::string path);

Camera myCam(glm::vec3(2.0f, 7.0f, 6.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

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
		std::cout << "Failed to create window." << std::endl;
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
		std::cout << "Failed to initialze GLAD." << std::endl;
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
		std::cout << "myShader program invalid!" << std::endl;
		return -1;
	}

	// ���lampShader������Ч��
	if (lampShader.Use() == false)
	{
		std::cout << "lampShader program invalid!" << std::endl;
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



		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Test Parameter");

		//camera
		ImGui::Text("CameraX %f | CameraY %f | CameraZ %f | CameraPitch %f | CameraYaw %f | CameraFov %f",
			myCam.camPos.x, myCam.camPos.y, myCam.camPos.z, myCam.pitchValue, myCam.yawValue, myCam.fov);

		// clear color
		static glm::vec3 bkgColor = glm::vec3(0.0f, 0.0f, 0.0f);
		ImGui::ColorEdit3("background", (float*)&bkgColor);
		glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// direction light
		static glm::vec3 dirLight_direction = glm::vec3(-1.0f, -1.0f, -1.0f);
		static glm::vec3 dirLight_ambient = glm::vec3(0.2f);
		static glm::vec3 dirLight_diffuse = glm::vec3(0.8f);
		static glm::vec3 dirLight_specular = glm::vec3(1.0f);
		ImGui::ColorEdit3("dirLight direction", (float*)&dirLight_direction);
		ImGui::ColorEdit3("dirLight ambient", (float*)&dirLight_ambient);
		ImGui::ColorEdit3("dirLight diffuse", (float*)&dirLight_diffuse);
		ImGui::ColorEdit3("dirLight specular", (float*)&dirLight_specular);

		// point light
		static float posValue = 0.0f;
		static glm::vec3 pointLight_ambient = glm::vec3(0.2f);
		static glm::vec3 pointLight_diffuse = glm::vec3(0.8f);
		static glm::vec3 pointLight_specular = glm::vec3(1.0f);
		float constant = 1.0f; // ͨ������1������
		float linear = 0.09f;
		float quadratic = 0.032f;
		const char* itemArray[] = { "										50", 
									"										100", 
									"										200", 
									"										600" };
		static int item = 0;
		ImGui::SliderFloat("pointLight position", &posValue, 0.0f, 10.0f);
		ImGui::ColorEdit3("pointLight ambient", (float*)&pointLight_ambient);
		ImGui::ColorEdit3("pointLight diffuse", (float*)&pointLight_diffuse);
		ImGui::ColorEdit3("pointLight specular", (float*)&pointLight_specular);
		ImGui::Combo("Light Fade Distance", &item, itemArray, IM_ARRAYSIZE(itemArray));

		switch (item)
		{
			case 0:
			{
				linear = 0.09f;
				quadratic = 0.032f;
				break;
			}
			case 1:
			{
				linear = 0.045f;
				quadratic = 0.0075f;
				break;
			}
			case 2:
			{
				linear = 0.022f;
				quadratic = 0.0019f;
				break;
			}
			case 3:
			{
				linear = 0.007f;
				quadratic = 0.0002f;
				break;
			}
			default:
			{
				std::cout << "Light Fade Distance Error!" << std::endl;
				break;
			}
				
		}

		// spotLight
		static glm::vec3 spotLight_ambient = glm::vec3(0.2f);
		static glm::vec3 spotLight_diffuse = glm::vec3(0.8f);
		static glm::vec3 spotLight_specular = glm::vec3(1.0f);
		static float spotLight_innerCos = 5.0f;
		static float spotLight_outerCos = 8.0f;
		ImGui::ColorEdit3("spotLight ambient", (float*)&spotLight_ambient);
		ImGui::ColorEdit3("spotLight diffuse", (float*)&spotLight_diffuse);
		ImGui::ColorEdit3("spotLight specular", (float*)&spotLight_specular);
		ImGui::SliderFloat("spotLight innerCos", &spotLight_innerCos, 0.0f, 90.0f);
		ImGui::SliderFloat("spotLight outerCos", &spotLight_outerCos, 0.0f, 90.0f);

		//material
		static int material_shininess = 32;
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

		myShader.SetVec3("dirLight.direction", glm::vec3(-1.0f, -1.0f, -1.0f));
		myShader.SetVec3("dirLight.ambient", dirLight_ambient);
		myShader.SetVec3("dirLight.diffuse", dirLight_diffuse);
		myShader.SetVec3("dirLight.specular", dirLight_specular);
		myShader.SetVec3("spotLight.lightPos", myCam.camPos);
		myShader.SetVec3("spotLight.direction", myCam.camFront);
		myShader.SetVec3("spotLight.ambient", spotLight_ambient);
		myShader.SetVec3("spotLight.diffuse", spotLight_diffuse);
		myShader.SetVec3("spotLight.specular", spotLight_specular);
		myShader.SetFloat("spotLight.innerCos", cos(glm::radians(spotLight_innerCos)));
		myShader.SetFloat("spotLight.outerCos", cos(glm::radians(spotLight_outerCos)));

		for (int i = 0; i < 4; i++)
		{
			std::stringstream ss;
			ss << "pointLight[" << i << "].";
			std::string prefix = ss.str();

			glm::vec3 lightPos = glm::vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));
			myShader.SetVec3(prefix + "lightPos", lightPos);
			myShader.SetVec3(prefix + "ambient", pointLight_ambient);
			myShader.SetVec3(prefix + "diffuse", pointLight_diffuse);
			myShader.SetVec3(prefix + "specular", pointLight_specular);
			myShader.SetFloat(prefix + "constant", 1.0f);
			myShader.SetFloat(prefix + "linear", linear);
			myShader.SetFloat(prefix + "quadratic", quadratic);
		}

		myShader.SetInt("material.shininess", material_shininess);

		glBindVertexArray(VAO); // draw������VAO�����Ķ�    �ɴ���VBO EBO attrpoint�İ󶨲������������

		// ���Դ�,�Ϳ��Խ��ж�д����������Ҫ����д�����Դ��ʱ��û�취ͬʱ��ͬһ��GL_TEXTURE_2D��ֻ��������Ԫ������
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_diffuse);  //Ƭ����ɫ�������GL_TEXTURE0��ȡtexture_diffuse����ͼ����
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_specular); //Ƭ����ɫ�������GL_TEXTURE1��ȡtexture_specular����ͼ����

		// view���� world -> view
		glm::mat4 view;
		myCam.setCamView();
		view = myCam.getCamView();
		myShader.SetMat4("uni_view", view);

		// ͶӰ���� view -> clip
		glm::mat4 projection;
		float fov = myCam.getCamFov();

		projection = glm::perspective(glm::radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f); // ֮ǰд��(float)(WINDOW_WIDTH / WINDOW_HEIGHT)�ˣ����ȶ�ʧ�����½����1
		myShader.SetMat4("uni_projection", projection);

		// model���� local -> world

		glm::mat4 model = glm::mat4(1.0f); // glm::mat4��ʼ�������ʾ���ó�ʼ��Ϊ��λ������Ϊ�°汾glm::mat4 model������ȫ0����
		myShader.SetMat4("uni_model", model);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		// ���
		glBindVertexArray(0);

		//��Դģ�ͣ�һ����ɫ�ķ�����
		//����lampShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
		lampShader.Use();

		glBindVertexArray(lightVAO);

		lampShader.SetMat4("uni_view", view);
		lampShader.SetMat4("uni_projection", projection);
		lampShader.SetVec3("uni_lightColor", glm::vec3(1.0f));

		for (int i = 0; i < 4; i++)
		{
			std::stringstream ss;
			ss << "pointLight[" << i << "].";
			std::string prefix = ss.str();

			glm::vec3 lightPos = glm::vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));

			glm::mat4 model = glm::mat4(1.0f); // ��ʼ��Ϊ��λ�������
			model = scale(model, glm::vec3(0.5f));
			model = translate(model, lightPos);
			model = rotate(model, glm::radians(45.0f + i * 10), glm::vec3(1.0f, 1.0f, 0.0f));

			lampShader.SetMat4("uni_model", model);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

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

bool LoadTexture(const std::string&& filePath, GLuint& texture)
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
		std::cout << "Failed to load texture��" << std::endl;
		return false;
	}
	// ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
	stbi_image_free(data);

	return true;
}

void loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	//directory = path.substr(0, path.find_last_of('/'));

	//processNode(scene->mRootNode, scene);
}