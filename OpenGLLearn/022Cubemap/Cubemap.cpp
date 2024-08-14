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

/************** Imgui���� **************/
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
/************** Imgui���� **************/

GLuint fbo = 0; // �Զ���֡�������
GLuint tbo = 0; // ��������󣨸�����
GLuint rbo = 0; // ��Ⱦ������󣨸�����

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
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "koalahjf", NULL, NULL);
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

	// ����shader ��������ȫ�ֱ�������Ϊshader����ز���������glfw��ʼ����ɺ�
	Shader myShader("Shader.vs", "Shader.fs");
	Shader screenShader("ShaderPostProcess.vs", "ShaderPostProcess.fs");
	Shader cubemapShader("ShaderCubemap.vs", "ShaderCubemap.fs");

	/* ������ͼ */
    // ��תy�ᣬʹͼƬ��opengl����һ��  �������assimp ����ģ��ʱ������aiProcess_FlipUVs���Ͳ����ظ�������
	stbi_set_flip_vertically_on_load(true);

	// ������ͼ
	GLuint t_metal = 0;
	GLuint t_marble = 0;
	GLuint t_dummy = 0;
	GLuint t_window = 0;

	LoadTexture("metal.png", t_metal, GL_REPEAT, GL_REPEAT);
	LoadTexture("marble.jpg", t_marble, GL_REPEAT, GL_REPEAT);
	LoadTexture("dummy_specular.png", t_dummy, GL_REPEAT, GL_REPEAT);  //�Լ�����ռλ��ͼ��ռһ��samplerλ�ã�����ᱻ����mesh�ĸ߹���ͼ���
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

	// ʵ�ʲ����Ǵ��Զ��建����������������ģ���˿�ʼ���dummyֻ��Ϊ�˸��ô��뷽��
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

		GetImguiValue();

		//other
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		SetValueToShader(myShader);
		SetValueToShader(screenShader);
		SetValueToShader(cubemapShader);

		/********************** �����Զ���֡�������������Ⱦ **********************/ 
		// �󶨵��Զ���֡���壬Ĭ��֡���岻��������
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// ��ո���������
		glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ���Ƶذ�
		//plane.DrawMesh(myShader);

		glEnable(GL_CULL_FACE); 
		// ��������������
		cube.SetScale(vec3(CUBE_SCALE_DEFAULT));
		cube.SetTranslate(vec3(1.0f, 1.5f, 1.0f));
		cube.DrawMesh(myShader);
		cube.SetTranslate(vec3(0.0f, 1.5f, -1.0f));
		cube.DrawMesh(myShader);

		// ��������
		nanosuit.SetScale(vec3(0.1f));
		nanosuit.DrawModel(myShader);

		glDisable(GL_CULL_FACE);
		// ������պ�
		skybox.DrawMesh(cubemapShader);

		glEnable(GL_CULL_FACE);

		// ���������������ľ�������mapĬ������������Ҳ���Ǵӽ���Զ
		// �������render loop���Ϊ�������ʵʱ�ı��
		map<float, vec3> sorted;
		for (int i = 0; i < squarePositions.size(); i++)
		{
			float distance = length(myCam.camPos - squarePositions[i]);
			sorted[distance] = squarePositions[i];
		}
		// ͸��������������ƣ�����͸������֮��Ҫ��Զ��������
		for (map<float, vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
		{
			square.SetTranslate(it->second);
			square.DrawMesh(myShader);
		}

		/********************** Ĭ��֡�������ǰ�����ʱд�� **********************/
		// �ص��Զ��建��Ķ�д�����л�����Ĭ�ϻ���
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		
		// ��ո���������
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		const vector<Texture> screenTexture =
		{
			{tbo, "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		screen.SetTextures(screenTexture);
		screen.DrawMesh(screenShader);

		// imgui��Ĭ�ϻ����л��ƣ���Ϊ�Ҳ���imguiҲ�к��ڴ���Ч��
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ���������� ��ѯ�¼�
		glfwSwapBuffers(window);
	}

	// ��Դ����
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

bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t)
{
	// �����Դ�ռ䲢��GL_TEXTURE_2D����
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // �󶨲���Ҫô�Ƕ�Ҫô��д��������Ҫд
	// ����GL_TEXTURE_2D�Ļ��ƣ����˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param_t);
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

	// ��д������ص���дȨ�޺��Ǹ���ϰ�ߣ�һֱ�������׳�bug
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
	//����myShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
	shader.Use();

	float constant = 1.0f; // ͨ������1������
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

	//���λ����Ҫʵʱ���µģ�����������shader1֮����������shader2��shader1�����û���Ч��
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

	// view���� world -> view
	mat4 view;
	myCam.setCamView();
	view = myCam.getCamView();
	shader.SetMat4("uni_view", view);

	// ͶӰ���� view -> clip
	mat4 projection;
	float fov = myCam.getCamFov();

	projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, myCam.camNear, myCam.camFar); // ֮ǰд��(float)(WINDOW_WIDTH / WINDOW_HEIGHT)�ˣ����ȶ�ʧ�����½����1
	shader.SetMat4("uni_projection", projection);

	// model���� local -> world
	mat4 model = mat4(1.0f); // mat4��ʼ�������ʾ���ó�ʼ��Ϊ��λ������Ϊ�°汾mat4 model������ȫ0����
	shader.SetMat4("uni_model", model);
}

//�����Զ���֡����
void CreateFrameBuffer()
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT0���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// ��Ⱦ������� ��Ϊһ��GL_DEPTH_STENCIL_ATTACHMENT���� ���ӵ� ֡������
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	
	// ���֡�������������
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//ɾ���Զ���֡����
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

	// ��������Ŀ��ĵĻ��ƣ����˷�ʽ
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < cubemapFaces.size(); i++)
	{
		// ��Ӳ�̼�����ͼ��ת��Ϊ�������ݣ��ȷŵ��ڴ棩
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
			// ��ͼ���� �ڴ� -> �Դ�
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			cout << "Failed to load cubemap��" << endl;
		}
		// ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
		stbi_image_free(data);
	}

	// ��д������ص���дȨ�޺��Ǹ���ϰ�ߣ�һֱ�������׳�bug
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	return cmo;
}