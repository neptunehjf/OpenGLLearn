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
#include "common.h"
#include <map>

#define CHARACTRER_SCALE_DEFAULT 0.1f
#define CHARACTRER_SCALE_OUTLINE 0.103f
#define CUBE_SCALE_DEFAULT 1.0f
#define CUBE_SCALE_OUTLINE 1.05f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t);
void GetImguiValue();
void SetValueToShader(Shader& shader);
void CreateFrameBuffer();
void DeleteFrameBuffer();
GLuint LoadCubemap(const vector<string>& cubemapFaces);

Camera myCam(vec3(1.5f, 2.0f, 3.8f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

/************** Imgui变量 **************/
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
static int postProcessType = 0;
static float sampleOffsetBase = 300.0f;
static float imgui_speed = 5.0f;
static float imgui_camNear = 0.1f;
static float imgui_camFar = 100.0f;
/************** Imgui变量 **************/

GLuint fbo = 0; // 自定义帧缓冲对象
GLuint tbo = 0; // 纹理缓冲对象（附件）
GLuint rbo = 0; // 渲染缓冲对象（附件）

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
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "koalahjf", NULL, NULL);
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

	// 创建shader 不能声明全局变量，因为shader的相关操作必须在glfw初始化完成后
	Shader myShader("Shader.vs", "Shader.fs");
	Shader screenShader("ShaderPostProcess.vs", "ShaderPostProcess.fs");
	Shader cubemapShader("ShaderCubemap.vs", "ShaderCubemap.fs");

	/* 加载贴图 */
    // 翻转y轴，使图片和opengl坐标一致  但是如果assimp 导入模型时设置了aiProcess_FlipUVs，就不能重复设置了
	stbi_set_flip_vertically_on_load(true);

	// 加载贴图
	GLuint t_metal = 0;
	GLuint t_marble = 0;
	GLuint t_dummy = 0;
	GLuint t_window = 0;

	LoadTexture("metal.png", t_metal, GL_REPEAT, GL_REPEAT);
	LoadTexture("marble.jpg", t_marble, GL_REPEAT, GL_REPEAT);
	LoadTexture("dummy_specular.png", t_dummy, GL_REPEAT, GL_REPEAT);  //自己做的占位贴图，占一个sampler位置，否则会被其他mesh的高光贴图替代
	LoadTexture("window.png", t_window, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	stbi_set_flip_vertically_on_load(false);
	const vector<string> cubemapFaces = {
	"Resource/Texture/skybox/left.jpg",
	"Resource/Texture/skybox/right.jpg",
	"Resource/Texture/skybox/top.jpg",
	"Resource/Texture/skybox/bottom.jpg",
	"Resource/Texture/skybox/back.jpg",
	"Resource/Texture/skybox/front.jpg"
	};
	GLuint t_cubemap = LoadCubemap(cubemapFaces);

	const vector<Texture> planeTexture =
	{
		{t_metal, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

	const vector<Texture> cubeTexture =
	{
		{t_marble, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

	const vector<Texture> windowTexture =
	{
		{t_window, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

	const vector<Texture> skyboxTexture =
	{
		{t_cubemap, "cubemap"}
	};

	Mesh plane(g_planeVertices, g_planeIndices, planeTexture);
	plane.SetScale(vec3(100.0f, 0.0f, 100.0f));
	Mesh cube(g_cubeVertices, g_cubeIndices, cubeTexture);
	Model nanosuit("nanosuit/nanosuit.obj");
	nanosuit.SetTranslate(vec3(1.0f, 1.0f, 0.0f));

	Mesh square(g_squareVertices, g_squareIndices, windowTexture);
	vector<vec3> squarePositions;
	squarePositions.push_back(glm::vec3(-1.5f, 1.0f, -0.48f));
	squarePositions.push_back(glm::vec3(1.5f, 1.0f, 0.51f));
	squarePositions.push_back(glm::vec3(0.0f, 1.0f, 0.7f));
	squarePositions.push_back(glm::vec3(-0.3f, 1.0f, -2.3f));
	squarePositions.push_back(glm::vec3(0.5f, 1.0f, -0.6f));

	Mesh skybox(g_skyboxVertices, g_skyboxIndices, skyboxTexture);

	// 实际材质是从自定义缓冲的纹理附件读出来的，因此开始设成dummy只是为了复用代码方便
	const vector<Texture> screenTexture =
	{
		{t_dummy, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};
	Mesh screen(g_screenVertices, g_screenIndices, screenTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	CreateFrameBuffer();

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

		GetImguiValue();

		//other
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		SetValueToShader(myShader);
		SetValueToShader(screenShader);
		SetValueToShader(cubemapShader);

		/********************** 先用自定义帧缓冲进行离屏渲染 **********************/ 
		// 绑定到自定义帧缓冲，默认帧缓冲不再起作用
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// 清空各个缓冲区
		glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 绘制地板
		//plane.DrawMesh(myShader);

		glEnable(GL_CULL_FACE); 
		// 绘制两个立方体
		cube.SetScale(vec3(CUBE_SCALE_DEFAULT));
		cube.SetTranslate(vec3(1.0f, 1.5f, 1.0f));
		cube.DrawMesh(myShader);
		cube.SetTranslate(vec3(0.0f, 1.5f, -1.0f));
		cube.DrawMesh(myShader);

		// 绘制人物
		nanosuit.SetScale(vec3(0.1f));
		nanosuit.DrawModel(myShader);

		glDisable(GL_CULL_FACE);
		// 绘制天空盒
		skybox.DrawMesh(cubemapShader);

		glEnable(GL_CULL_FACE);

		// 按窗户离摄像机间的距离排序，map默认是升序排序，也就是从近到远
		// 必须放在render loop里，因为摄像机是实时改变的
		map<float, vec3> sorted;
		for (int i = 0; i < squarePositions.size(); i++)
		{
			float distance = length(myCam.camPos - squarePositions[i]);
			sorted[distance] = squarePositions[i];
		}
		// 透明物体必须最后绘制，并且透明物体之间要从远到近绘制
		for (map<float, vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
		{
			square.SetTranslate(it->second);
			square.DrawMesh(myShader);
		}

		/********************** 默认帧缓冲输出前面绘制时写入 **********************/
		// 关掉自定义缓冲的读写，就切换成了默认缓冲
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		
		// 清空各个缓冲区
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		const vector<Texture> screenTexture =
		{
			{tbo, "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		screen.SetTextures(screenTexture);
		screen.DrawMesh(screenShader);

		// imgui在默认缓冲中绘制，因为我不想imgui也有后期处理效果
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 缓冲区交换 轮询事件
		glfwSwapBuffers(window);
	}

	// 资源清理
	nanosuit.DeleteModel();
	plane.DeleteMesh();
	cube.DeleteMesh();
	skybox.DeleteMesh();
	square.DeleteMesh();
	screen.DeleteMesh();
	myShader.Remove();
	screenShader.Remove();
	cubemapShader.Remove();
	DeleteFrameBuffer();
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

bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t)
{
	// 申请显存空间并绑定GL_TEXTURE_2D对象
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // 绑定操作要么是读要么是写，这里是要写
	// 设置GL_TEXTURE_2D的环绕，过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载贴图，转换为像素数据
	int width, height, channel;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channel, 0);

	GLenum format = 0;
	if (channel == 1)
		format = GL_RED;
	else if (channel == 3)
		format = GL_RGB;
	else if (channel == 4)
		format = GL_RGBA;

	if (data)
	{
		// 贴图数据 内存 -> 显存
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		// 生成多级渐进贴图
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture！" << endl;
		return false;
	}
	// 像素数据已经传给显存了，删除内存中的像素数据
	stbi_image_free(data);

	// 读写结束后关掉独写权限后是个好习惯，一直开着容易出bug
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void GetImguiValue()
{
	if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("CameraX %f | CameraY %f | CameraZ %f \n CameraPitch %f | CameraYaw %f | CameraFov %f",
			myCam.camPos.x, myCam.camPos.y, myCam.camPos.z, myCam.pitchValue, myCam.yawValue, myCam.fov);

		ImGui::SliderFloat("Movement Speed", &imgui_speed, 0.0f, 100.0f);
		myCam.camSpeed = imgui_speed;

		ImGui::SliderFloat("View Near", &imgui_camNear, 0.1f, 1000.0f);
		myCam.camNear = imgui_camNear;

		ImGui::SliderFloat("View Far", &imgui_camFar, 0.1f, 1000.0f);
		myCam.camFar = imgui_camFar;

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Lighting"))
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
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("PostProcess", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* itemArray[] = {"Default", "Sharpen", "Edge Detection", "Blur"};
		ImGui::Combo("PostProcess Type", &postProcessType, itemArray, IM_ARRAYSIZE(itemArray));

		ImGui::SliderFloat("PostProcess sample offset", &sampleOffsetBase, 1.0f, 3000.0f);
		ImGui::TreePop();
	}
}

void SetValueToShader(Shader& shader)
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
	shader.SetInt("kernel_type", postProcessType);
	shader.SetFloat("sample_offset_base", sampleOffsetBase);
	
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

	projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, myCam.camNear, myCam.camFar); // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
	shader.SetMat4("uni_projection", projection);

	// model矩阵 local -> world
	mat4 model = mat4(1.0f); // mat4初始化最好显示调用初始化为单位矩阵，因为新版本mat4 model可能是全0矩阵
	shader.SetMat4("uni_model", model);
}

//创建自定义帧缓冲
void CreateFrameBuffer()
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// 只有默认缓冲才能输出图像，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// 生成纹理附件 对应color缓冲
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT0附件 附加到 帧缓冲对象
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);

	// 生成渲染缓冲对象 对应stencil，depth缓冲
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 渲染缓冲对象 作为一个GL_DEPTH_STENCIL_ATTACHMENT附件 附加到 帧缓冲上
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	
	// 检查帧缓冲对象完整性
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//删除自定义帧缓冲
void DeleteFrameBuffer()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteFramebuffers(1, &tbo);
	glDeleteFramebuffers(1, &rbo);
}

GLuint LoadCubemap(const vector<string>& cubemapFaces)
{
	GLuint cmo = 0;
	glGenTextures(1, &cmo);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cmo);

	// 设置纹理目标的的环绕，过滤方式
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < cubemapFaces.size(); i++)
	{
		// 从硬盘加载贴图，转换为像素数据（先放到内存）
		int width, height, channel;
		unsigned char* data = stbi_load(cubemapFaces[i].c_str(), &width, &height, &channel, 0);

		GLenum format = 0;
		if (channel == 1)
			format = GL_RED;
		else if (channel == 3)
			format = GL_RGB;
		else if (channel == 4)
			format = GL_RGBA;

		if (data)
		{
			// 贴图数据 内存 -> 显存
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			cout << "Failed to load cubemap！" << endl;
		}
		// 像素数据已经传给显存了，删除内存中的像素数据
		stbi_image_free(data);
	}

	// 读写结束后关掉独写权限后是个好习惯，一直开着容易出bug
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	return cmo;
}