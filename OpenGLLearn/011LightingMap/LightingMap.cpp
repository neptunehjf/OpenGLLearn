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

	// 初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 绘制窗口
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenglWindow", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();  
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 捕获鼠标
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

	// 创建Shader程序
	Shader myShader("shader.vs", "shader.fs");
	Shader lampShader("lampShader.vs", "lampShader.fs"); // 灯本身shader，一个白色的发光体

	// 用显存VAO来管理 shader的顶点属性
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // VBO glVertexAttribPointer 操作向VAO上下文写

	// 存储顶点数据到显存VBO
	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// 定义顶点属性的解析方式
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(0 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(0);
	//3 忘记加sizeof(GL_FLOAT)了，排查了半天。。。以后0也写成0 * sizeof(GL_FLOAT)的形式吧。。以免误导别的代码
	glVertexAttribPointer(1, NORMAL_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT))); 
	glEnableVertexAttribArray(1);

	// 存储下标数据到显存EBO
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文

	// 专门为光源定义了一个VAO，方便后续操作
	GLuint lightVAO = 0;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO); // VBO glVertexAttribPointer 操作向VAO上下文写
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(0 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//当目标是GL_ELEMENT_ARRAY_BUFFER的时候，VAO会储存glBindBuffer的函数调用。这也意味着它也会储存解绑调用，所以确保你没有在解绑VAO之前解绑索引数组缓冲，否则它就没有这个EBO配置了
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// 检查myShader程序有效性
	if (myShader.Use() == false)
	{
		cout << "myShader program invalid!" << endl;
		return -1;
	}

	// 检查lampShader程序有效性
	if (lampShader.Use() == false)
	{
		cout << "lampShader program invalid!" << endl;
		return -1;
	}

	// 开启深度测试
	glEnable(GL_DEPTH_TEST);

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		//输入
		processInput(window);
		glfwPollEvents();

		// 清空buffer
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

		//激活myShader程序 这里涉及两个shader程序的切换，所以每次loop里都要在对应的位置调用，不能只在开始调用一次
		myShader.Use();

		myShader.SetVec3("uni_viewPos", myCam.camPos); //相机位置是实时更新的

		/* 生成变换矩阵 */
		// view矩阵 world -> view
		mat4 view;

		myCam.setCamView();
		view = myCam.getCamView();
		myShader.SetMat4("uni_view", view);

		// 投影矩阵 view -> clip
		mat4 projection;
		float fov = myCam.getCamFov();

		projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f); // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
		myShader.SetMat4("uni_projection", projection);

		// model矩阵 local -> world
		// 物体
		mat4 model = mat4(1.0f); // mat4初始化最好显示调用初始化为单位矩阵，因为新版本mat4 model可能是全0矩阵
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

		glBindVertexArray(VAO); // draw操作从VAO上下文读    可代替VBO EBO attrpoint的绑定操作，方便管理
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		// 解绑
		glBindVertexArray(0);

		vec3 lightPos = vec3(5 * cos(ligthShift), 10.0f, 5 * sin(ligthShift));
		myShader.SetVec3("light.lightPos", lightPos);
		// 光源模型，一个白色的发光体
		// 激活lampShader程序 这里涉及两个shader程序的切换，所以每次loop里都要在对应的位置调用，不能只在开始调用一次
		lampShader.Use();

		model = mat4(1.0f); // 初始化为单位矩阵，清空
		model = scale(model, vec3(0.5f));
		model = translate(model, lightPos);
		model = rotate(model, radians(45.0f), vec3(1.0f, 1.0f, 0.0f));

		lampShader.SetMat4("uni_view", view);
		lampShader.SetMat4("uni_projection", projection);
		lampShader.SetMat4("uni_model", model);
		lampShader.SetVec3("uni_lightColor", vec3(1.0f));

		glBindVertexArray(lightVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// 解绑
		glBindVertexArray(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 缓冲区交换 轮询事件
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
	/* 相机平移 */
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
	/* 相机视角 */

	float offsetX = posX - myCam.lastX;
	float offsetY = myCam.lastY - posY;
	myCam.lastX = posX;
	myCam.lastY = posY;

	// 鼠标右键不按就不处理，因为鼠标要用来点Imgui
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
	/* 镜头缩放 */
	if (myCam.fov >= 1.0f && myCam.fov <= 95.0f)
		myCam.fov -= yoffset;
	if (myCam.fov <= 1.0f)
		myCam.fov = 1.0f;
	if (myCam.fov >= 95.0f)
		myCam.fov = 95.0f;
}

// 翻转y轴，使图片和opengl坐标一致
stbi_set_flip_vertically_on_load(true);

bool LoadTexure(const string&& filePath, GLuint& texture)
{
	// 申请显存空间并绑定GL_TEXTURE_2D对象
	glGenTextures(1, &texture); //texture是引用的话，这里用直接用texture就行了吧？
	glBindTexture(GL_TEXTURE_2D, texture); // 绑定操作要么是读要么是写，这里是要写
	// 设置GL_TEXTURE_2D的环绕，过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载贴图，转换为像素数据
	int width, height, channel;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channel, 0);
	if (data)
	{
		// 显存生成贴图数据
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture0！" << endl;
		return false;
	}
	// 像素数据已经传给显存了，删除内存中的像素数据
	stbi_image_free(data);

	return true;
}