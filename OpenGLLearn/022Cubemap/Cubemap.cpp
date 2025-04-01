﻿#include "glad/glad.h"
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
void CreateFrameBuffer(GLuint& fbo, GLuint& tbo, GLuint& rbo);
void DeleteFrameBuffer();
GLuint LoadCubemap(const vector<string>& cubemapFaces);

Camera myCam(vec3(1.5f, 2.0f, 3.8f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
//Shader test("shader.vs", "shader.fs");  不能声明全局变量，因为shader要调用glfw的api，所以必须在glfw初始化完成后
// Shaderグローバル変数禁止 コンストラクタはglfw初期化後に呼び出し必須 (glfwのapiが必要のため)

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


int main()
{
	char infoLog[LOG_LENGTH] = "\0";

	// 初期化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ウィンドウを作成する
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
	// マウスキャプチャ
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
	// シェーダープログラムを作成
	Shader lightShader("ShaderLighting.vs", "ShaderLighting.fs");
	Shader screenShader("ShaderPostProcess.vs", "ShaderPostProcess.fs");
	Shader cubemapShader("ShaderCubemap.vs", "ShaderCubemap.fs");
	Shader reflectShader("ShaderReflection.vs", "ShaderReflection.fs");
	Shader refractShader("ShaderRefraction.vs", "ShaderRefraction.fs");

	// 翻转y轴，使图片坐标和opengl坐标一致
	// Y軸を反転して画像座標とOpenGL座標を一致させる
	// ※AssimpのaiProcess_FlipUVsフラグ使用時は二重設定禁止
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
		{t_cubemap, "texture_cubemap"}
	};

	Mesh plane(g_planeVertices, g_planeIndices, planeTexture);
	plane.SetScale(vec3(100.0f, 0.0f, 100.0f));
	Mesh cube(g_cubeVertices, g_cubeIndices, skyboxTexture);
	Model nanosuit("nanosuit_reflection/nanosuit.obj");
	nanosuit.SetTranslate(vec3(1.0f, 1.0f, 0.0f));

	// 赋值号，默认vector是深拷贝，因此SetTextures不会影响nanosuit对象
	// 代入演算子、vectorはデフォルトでディープコピーを行うためSetTexturesはnanosuitオブジェクトに影響しない
	//vector<Mesh> suitMeshes = nanosuit.meshes;  
	
	// 使用引用，引用只是nanosuit.meshes的别名，因此SetTextures会影响到nanosuit对象
	// 参照を使用（エイリアス）、SetTexturesの変更はnanosuitオブジェクトに反映される
	vector<Mesh>& suitMeshes = nanosuit.GetMeshes(); 
						
	for (unsigned int i = 0; i < suitMeshes.size(); i++)
	{
		suitMeshes[i].AddTextures(skyboxTexture);
	}

	Mesh square(g_squareVertices, g_squareIndices, windowTexture);
	vector<vec3> squarePositions;
	squarePositions.push_back(glm::vec3(-1.5f, 1.0f, -0.48f));
	squarePositions.push_back(glm::vec3(1.5f, 1.0f, 0.51f));
	squarePositions.push_back(glm::vec3(0.0f, 1.0f, 0.7f));
	squarePositions.push_back(glm::vec3(-0.3f, 1.0f, -2.3f));
	squarePositions.push_back(glm::vec3(0.5f, 1.0f, -0.6f));

	Mesh skybox(g_skyboxVertices, g_skyboxIndices, skyboxTexture);

	// 实际材质是从自定义缓冲的纹理附件读出来的，因此开始设成dummy只是为了复用代码方便
	// 実際のマテリアルはカスタムバッファのテクスチャアタッチメントから読み込むため、初期のダミー設定はコード再利用のため
	const vector<Texture> screenTexture =
	{
		{t_dummy, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};
	Mesh screen(g_screenVertices, g_screenIndices, screenTexture);

	const vector<Texture> mirrorTexture =
	{
		{t_dummy, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};
	Mesh mirror(g_mirrorVertices, g_mirrorIndices, mirrorTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// 原场景
	// 原シーン
	GLuint fbo1 = 0;
	GLuint tbo1 = 0;
	GLuint rbo1 = 0;
	CreateFrameBuffer(fbo1, tbo1, rbo1);

	// 后视镜缓冲
	// バックミラー
	GLuint fbo2 = 0; 
	GLuint tbo2 = 0; 
	GLuint rbo2 = 0; 
	CreateFrameBuffer(fbo2, tbo2, rbo2);

	//渲染循环
	//　レンダリングループ
	while (!glfwWindowShouldClose(window))
	{
		//入力
		glfwPollEvents();
		processInput(window);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Test Parameter");

		GetImguiValue();

		//other
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);

		/********************** 先用自定义帧缓冲进行离屏渲染 **********************/ 
		/********************** カスタムフレームバッファによるオフスクリーンレンダリング **********************/
		// 绑定到自定义帧缓冲，关闭对默认帧缓冲的读写
		// カスタムフレームバッファにバインド（デフォルトフレームバッファへの読み書きを無効化）

		for (unsigned int i = 1; i <= 2; i++)
		{
			if (i == 1)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, fbo1); 
			}
			else if (i == 2)
			{
				myCam.yawValue += 180.0;
				glBindFramebuffer(GL_FRAMEBUFFER, fbo2); 
			}

			SetValueToShader(lightShader);
			SetValueToShader(screenShader);
			SetValueToShader(cubemapShader);
			SetValueToShader(reflectShader);
			SetValueToShader(refractShader);

		// 清空各个缓冲区
		// 各バッファをクリア
			glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// 绘制地板
			// 床の描画
			//plane.DrawMesh(lightShader);

			// 绘制两个立方体
		    // 立方体2個を描画
			cube.SetScale(vec3(CUBE_SCALE_DEFAULT));
			cube.SetTranslate(vec3(1.0f, 1.5f, 1.0f));
			cube.DrawMesh(reflectShader);
			//cube.DrawMesh(refractShader);
			cube.SetTranslate(vec3(0.0f, 1.5f, -1.0f));
			cube.DrawMesh(refractShader);
			//cube.DrawMesh(reflectShader);

			glDisable(GL_BLEND);
			// 绘制人物
			// キャラクター描画
			nanosuit.SetScale(vec3(0.1f));
			nanosuit.DrawModel(lightShader);
			//nanosuit.DrawModel(refractShader);
			glEnable(GL_BLEND);

			glDisable(GL_CULL_FACE);
			// 绘制天空盒
			skybox.DrawMesh(cubemapShader);

			// 按窗户离摄像机间的距离排序，map默认是升序排序，也就是从近到远
			// 必须放在render loop里，因为摄像机是实时改变的
			// ウィンドウオブジェクトとカメラ間の距離でソート（std::mapのデフォルトは昇順=近→遠）
			// カメラ位置がリアルタイム更新されるため、レンダリングループ内で毎フレーム実行必須
			map<float, vec3> sorted;
			for (int i = 0; i < squarePositions.size(); i++)
			{
				float distance = length(myCam.camPos - squarePositions[i]);
				sorted[distance] = squarePositions[i];
			}
			// 透明物体必须最后绘制，并且透明物体之间要从远到近绘制
			// 透明オブジェクトは最終描画かつ、透明同士は遠→近の逆順描画が必要
			for (map<float, vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
			{
				square.SetTranslate(it->second);
				square.DrawMesh(lightShader);
			}

			if (i == 2)
			{
				myCam.yawValue -= 180.0;
				SetValueToShader(lightShader);
				SetValueToShader(screenShader);
				SetValueToShader(cubemapShader);
				SetValueToShader(reflectShader);
				SetValueToShader(refractShader);
			}
		}

		/********************** 绑定回默认帧缓冲 **********************/
		/********************** デフォルトフレームバッファへの再バインド **********************/

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); 
		const vector<Texture> screenTexture =
		{
			{tbo1, "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		screen.SetTextures(screenTexture);
		screen.DrawMesh(screenShader);

		const vector<Texture> mirrorTexture =
		{
			{tbo2, "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		mirror.SetTextures(mirrorTexture);
		mirror.DrawMesh(screenShader);

		// imgui在默认缓冲中绘制，因为我不想imgui也有后期处理效果
		// ImGUIのレンダリングはデフォルトフレームバッファで行う（ポストプロセス効果を適用しないため）
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	nanosuit.DeleteModel();
	plane.DeleteMesh();
	cube.DeleteMesh();
	skybox.DeleteMesh();
	square.DeleteMesh();
	screen.DeleteMesh();
	lightShader.Remove();
	screenShader.Remove();
	cubemapShader.Remove();
	//DeleteFrameBuffer();
	glDeleteFramebuffers(1, &fbo1);
	glDeleteFramebuffers(1, &tbo1);
	glDeleteFramebuffers(1, &rbo1);
	glDeleteFramebuffers(1, &fbo2);
	glDeleteFramebuffers(1, &tbo2);
	glDeleteFramebuffers(1, &rbo2);
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
	// カメラ移動

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
	// カメラ回転
	float offsetX = posX - myCam.lastX;
	float offsetY = myCam.lastY - posY;
	myCam.lastX = posX;
	myCam.lastY = posY;

	// 按住鼠标右键时才会移动相机
	// マウス右ボタン押下中のみカメラ回転を許可  
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
	/* FOV */
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
    // VRAM領域確保し、GL_TEXTURE_2Dオブジェクトをバインド
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); 
	// 设置GL_TEXTURE_2D的环绕，过滤方式
    // GL_TEXTURE_2Dのラップモードとフィルタリング設定
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载贴图，转换为像素数据
    // テクスチャ読み込み、データに変換
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
        /* テクスチャデータをVRAMに転送する */
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		// 生成多级渐进贴图
		// ミップマップを生成する  
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture！" << endl;
		return false;
	}
	// 数据已经传给显存了，删除内存中的数据
    // メモリ上のデータ削除（VRAMに転送完了後）
	stbi_image_free(data);

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
	// 2つのシェーダープログラムの切り替えが必要なため、ループ毎に対応位置で呼び出すこと（初期呼び出しのみ不適）
	shader.Use();

	float constant = 1.0f;
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
	// view行列（視点変換行列）
	mat4 view;
	myCam.setCamView();
	view = myCam.getCamView();
	shader.SetMat4("uni_view", view);

	// 投影矩阵 view -> clip
	// projection行列（射影変換行列）
	mat4 projection;
	float fov = myCam.getCamFov();

	 // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
	 // 	以前は(float)(WINDOW_WIDTH / WINDOW_HEIGHT)と記述していたため、整数除算で精度が失われ結果が1になっていた
	projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, myCam.camNear, myCam.camFar); 
	shader.SetMat4("uni_projection", projection);

	// model矩阵 local -> world
	// model行列（オブジェクト空間→ワールド空間変換）

	mat4 model = mat4(1.0f); // mat4初始化最好显示调用初始化为单位矩阵，因为新版本mat4 model可能是全0矩阵
	// mat4の初期化は明示的に単位行列で行うべき　最新バージョンでは暗黙的にゼロ行列が生成される可能性あり
	shader.SetMat4("uni_model", model);
}


void CreateFrameBuffer(GLuint& fbo, GLuint& tbo, GLuint& rbo)
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// フレームバッファオブジェクト（FBO）の生成（color stencil depthを含む。デフォルトバッファも同様だが、独自バッファを作成することで特殊効果が実現可能）
	// 只有默认缓冲才能输出图像，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	// デフォルトバッファのみが画面出力可能。自作バッファは画面表示されないため、オフスクリーンレンダリングに活用
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// 生成纹理附件 对应color缓冲
	// テクスチャアタッチメント生成（カラーバッファ対応）
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT0附件 附加到 帧缓冲对象
	// TBOをGL_COLOR_ATTACHMENT0としてFBOにアタッチ
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);

	// 生成渲染缓冲对象 对应stencil，depth缓冲
	// レンダーバッファオブジェクト(RBO)生成（ステンシル・深度バッファ対応）
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 渲染缓冲对象 作为一个GL_DEPTH_STENCIL_ATTACHMENT附件 附加到 帧缓冲上
	// RBOをGL_DEPTH_STENCIL_ATTACHMENTとしてFBOにアタッチ
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	
	// 检查帧缓冲对象完整性
	// フレームバッファの完全性チェック
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void DeleteFrameBuffer()
{
	//glDeleteFramebuffers(1, &fbo);
	//glDeleteFramebuffers(1, &tbo);
	//glDeleteFramebuffers(1, &rbo);
}

GLuint LoadCubemap(const vector<string>& cubemapFaces)
{
	// 申请显存空间并绑定GL_TEXTURE_2D对象
	// VRAM領域確保し、GL_TEXTURE_2Dオブジェクトをバインド
	GLuint cmo = 0;
	glGenTextures(1, &cmo);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cmo);

	// 设置GL_TEXTURE_2D的环绕，过滤方式
	// GL_TEXTURE_2Dのラップモードとフィルタリング設定

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < cubemapFaces.size(); i++)
	{
		// 加载贴图，转换为像素数据
		// テクスチャ読み込み、データに変換
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
		/* テクスチャデータをVRAMに転送する */
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			// 生成多级渐进贴图
			// ミップマップを生成する  
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			cout << "Failed to load cubemap！" << endl;
		}
		// 数据已经传给显存了，删除内存中的数据
		// メモリ上のデータ削除（VRAMに転送完了後）
		stbi_image_free(data);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return cmo;
}