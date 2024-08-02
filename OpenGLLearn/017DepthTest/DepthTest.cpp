#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>
#include <cstdio>

#include "Shader.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <string>
#include <sstream>

#include "Model.h"
#include "VertexData.h"
#include "namespace.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//bool LoadTexture(const string&& filePath, GLuint& texture);
void GetImguiValue();
void SetUniformValue(Shader& shader, Shader& shader_lamp);

Camera myCam(vec3(2.0f, 7.0f, 6.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
//Shader test("shader.vs", "shader.fs");  不能声明全局变量，因为shader的相关操作必须在glfw初始化完成后

static float posValue = 0.0f;
static vec3 bkgColor = vec3(0.0f, 0.0f, 0.0f);
static vec3 dirLight_direction = vec3(-1.0f, -1.0f, -1.0f);
static vec3 dirLight_ambient = vec3(0.2f);
static vec3 dirLight_diffuse = vec3(0.8f);
static vec3 dirLight_specular = vec3(1.0f);
static vec3 pointLight_ambient = vec3(0.2f);
static vec3 pointLight_diffuse = vec3(0.8f);
static vec3 pointLight_specular = vec3(1.0f);
static vec3 spotLight_ambient = vec3(0.2f);
static vec3 spotLight_diffuse = vec3(0.8f);
static vec3 spotLight_specular = vec3(1.0f);
static float spotLight_innerCos = 5.0f;
static float spotLight_outerCos = 8.0f;
static int item = 0;
static int material_shininess = 32;

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

	Shader myShader("shader.vs", "shader.fs");
	Shader lampShader("lampShader.vs", "lampShader.fs"); // 灯本身shader，一个白色的发光体

	/* 加载贴图 */
    // 翻转y轴，使图片和opengl坐标一致
	//stbi_set_flip_vertically_on_load(true);

	//// 加载贴图
	//GLuint container_diffuse = 0;
	//GLuint container_specular = 0;
	//bool ret1, ret2;
	//ret1 = LoadTexture("container_diffuse.png", container_diffuse);
	//ret2 = LoadTexture("container_specular.png", container_specular);
	//if (ret1 == false || ret2 == false)
	//	return -1;

	//const vector<Texture> textures =
	//{
	//	{container_diffuse, "texture_diffuse"},
	//	{container_specular, "texture_specular"}
	//};

	//Mesh mesh(g_vertices, g_indices, textures);
	//mesh.SetupMesh();

	Model model("nanosuit/nanosuit.obj");

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

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Test Parameter");

		//camera info
		ImGui::Text("CameraX %f | CameraY %f | CameraZ %f | CameraPitch %f | CameraYaw %f | CameraFov %f",
			myCam.camPos.x, myCam.camPos.y, myCam.camPos.z, myCam.pitchValue, myCam.yawValue, myCam.fov);

		GetImguiValue();

		//other
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		// clear color
		glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SetUniformValue(myShader, lampShader);

		model.DrawModel(myShader, lampShader, posValue);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 缓冲区交换 轮询事件
		glfwSwapBuffers(window);

	}

	model.DeleteModel();
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

//bool LoadTexture(const string&& filePath, GLuint& texture)
//{
//	// 申请显存空间并绑定GL_TEXTURE_2D对象
//	glGenTextures(1, &texture);
//	glBindTexture(GL_TEXTURE_2D, texture); // 绑定操作要么是读要么是写，这里是要写
//	// 设置GL_TEXTURE_2D的环绕，过滤方式
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	// 加载贴图，转换为像素数据
//	int width, height, channel;
//	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channel, 0);
//
//	GLenum format = 0;
//	if (channel == 1)
//		format = GL_RED;
//	else if (channel == 3)
//		format = GL_RGB;
//	else if (channel == 4)
//		format = GL_RGBA;
//
//
//	//printf("**********************************************************\n");
//	//for (int i = 1; i <= width * height; i++)
//	//{
//	//	printf("%02x ", data[i]);
//	//	if (i % width == 0)
//	//		printf("\n");
//	//}
//	//printf("**********************************************************\n");
//
//	if (data)
//	{
//		// 贴图数据 内存 -> 显存
//		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//		// 生成多级渐进贴图
//		glGenerateMipmap(GL_TEXTURE_2D);
//	}
//	else
//	{
//		cout << "Failed to load texture！" << endl;
//		return false;
//	}
//	// 像素数据已经传给显存了，删除内存中的像素数据
//	stbi_image_free(data);
//
//	return true;
//}

void GetImguiValue()
{
	// clear color
	ImGui::ColorEdit3("background", (float*)&bkgColor);

	// direction light
	ImGui::ColorEdit3("dirLight direction", (float*)&dirLight_direction);
	ImGui::ColorEdit3("dirLight ambient", (float*)&dirLight_ambient);
	ImGui::ColorEdit3("dirLight diffuse", (float*)&dirLight_diffuse);
	ImGui::ColorEdit3("dirLight specular", (float*)&dirLight_specular);

	// point light
	const char* itemArray[] = { "50", "100", "200","600" };
	ImGui::SliderFloat("pointLight position", &posValue, 0.0f, 10.0f);
	ImGui::ColorEdit3("pointLight ambient", (float*)&pointLight_ambient);
	ImGui::ColorEdit3("pointLight diffuse", (float*)&pointLight_diffuse);
	ImGui::ColorEdit3("pointLight specular", (float*)&pointLight_specular);
	ImGui::Combo("Light Fade Distance", &item, itemArray, IM_ARRAYSIZE(itemArray));

	// spotLight
	ImGui::ColorEdit3("spotLight ambient", (float*)&spotLight_ambient);
	ImGui::ColorEdit3("spotLight diffuse", (float*)&spotLight_diffuse);
	ImGui::ColorEdit3("spotLight specular", (float*)&spotLight_specular);
	ImGui::SliderFloat("spotLight innerCos", &spotLight_innerCos, 0.0f, 90.0f);
	ImGui::SliderFloat("spotLight outerCos", &spotLight_outerCos, 0.0f, 90.0f);

	//material
	ImGui::SliderInt("material shininess", &material_shininess, 0, 256);
}

void SetUniformValue(Shader& shader, Shader& shader_lamp)
{
	//激活myShader程序 这里涉及两个shader程序的切换，所以每次loop里都要在对应的位置调用，不能只在开始调用一次
	shader.Use();

	float constant = 1.0f; // 通常保持1就行了
	float linear = 0.09f;
	float quadratic = 0.032f;
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
		cout << "Light Fade Distance Error!" << endl;
		break;
	}

	}

	//相机位置是要实时更新的，而且启动了shader1之后又启动了shader2，shader1的设置会无效化
	shader.SetVec3("uni_viewPos", myCam.camPos);
	shader.SetVec3("dirLight.direction", vec3(-1.0f, -1.0f, -1.0f));
	shader.SetVec3("dirLight.ambient", dirLight_ambient);
	shader.SetVec3("dirLight.diffuse", dirLight_diffuse);
	shader.SetVec3("dirLight.specular", dirLight_specular);
	shader.SetVec3("spotLight.lightPos", myCam.camPos);
	shader.SetVec3("spotLight.direction", myCam.camFront);
	shader.SetVec3("spotLight.ambient", spotLight_ambient);
	shader.SetVec3("spotLight.diffuse", spotLight_diffuse);
	shader.SetVec3("spotLight.specular", spotLight_specular);
	shader.SetFloat("spotLight.innerCos", cos(radians(spotLight_innerCos)));
	shader.SetFloat("spotLight.outerCos", cos(radians(spotLight_outerCos)));
	shader.SetInt("material.shininess", material_shininess);
	for (int i = 0; i < 4; i++)
	{
		stringstream ss;
		ss << "pointLight[" << i << "].";
		string prefix = ss.str();

		vec3 lightPos = vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));
		shader.SetVec3(prefix + "lightPos", lightPos);
		shader.SetVec3(prefix + "ambient", pointLight_ambient);
		shader.SetVec3(prefix + "diffuse", pointLight_diffuse);
		shader.SetVec3(prefix + "specular", pointLight_specular);
		shader.SetFloat(prefix + "constant", 1.0f);
		shader.SetFloat(prefix + "linear", linear);
		shader.SetFloat(prefix + "quadratic", quadratic);
	}

	// view矩阵 world -> view
	mat4 view;
	myCam.setCamView();
	view = myCam.getCamView();
	shader.SetMat4("uni_view", view);

	// 投影矩阵 view -> clip
	mat4 projection;
	float fov = myCam.getCamFov();

	projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f); // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
	shader.SetMat4("uni_projection", projection);

	// model矩阵 local -> world

	mat4 model = mat4(1.0f); // mat4初始化最好显示调用初始化为单位矩阵，因为新版本mat4 model可能是全0矩阵
	shader.SetMat4("uni_model", model);

	shader_lamp.Use();

	shader_lamp.SetMat4("uni_view", view);
	shader_lamp.SetMat4("uni_projection", projection);
	shader_lamp.SetVec3("uni_lightColor", vec3(1.0f));
}