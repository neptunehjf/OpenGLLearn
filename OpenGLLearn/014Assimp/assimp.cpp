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

using namespace std;
using namespace glm;

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool LoadTexture(const string&& filePath, GLuint& texture);
void loadModel(string path);

Camera myCam(vec3(2.0f, 7.0f, 6.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

int main()
{
	char infoLog[LOG_LENGTH] = "\0";

	// 初期化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ウィンドウを作成する
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
	Shader myShader("shader.vs", "shader.fs");
	Shader lampShader("lampShader.vs", "lampShader.fs");

	// 翻转y轴，使图片坐标和opengl坐标一致
	// Y軸を反転して画像座標とOpenGL座標を一致させる
	stbi_set_flip_vertically_on_load(true);

	// 加载贴图
	// テクスチャを読み込む
	GLuint texture_diffuse = 0;
	GLuint texture_specular = 0;
	bool ret1, ret2;
	ret1 = LoadTexture("container_diffuse.png", texture_diffuse);
	ret2 = LoadTexture("container_specular.png", texture_specular);
	if (ret1 == false || ret2 == false)
		return -1;

	// 参照 Referrence/opengl vertex management.png
	// 用VAO来管理 shader的顶点属性
	// VAOを使用してシェーダーの頂点属性を管理
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// 存储顶点数据到VBO
	// 頂点データをVBOに格納	
	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(0 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(0);
	// 3忘记加sizeof(GL_FLOAT)了，排查了半天。。。以后0也写成0 * sizeof(GL_FLOAT)的形式吧。。以免误导别的代码
	// 3のsizeof(GL_FLOAT)を書き忘れて半日デバッグした…今後は0も「0 * sizeof(GL_FLOAT)」形式で書こう…他コードの誤解防止のため 
	glVertexAttribPointer(1, NORMAL_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT))); 
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);

	// 存储下标数据到EBO
	// インデックスデータをEBOに格納
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);

	// 表示光源的物体
	// 光源を表すオブジェクト
	GLuint lightVAO = 0;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO); 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(0 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// 有効チェック
	if (myShader.Use() == false)
	{
		cout << "myShader program invalid!" << endl;
		return -1;
	}

	// 有効チェック
	if (lampShader.Use() == false)
	{
		cout << "lampShader program invalid!" << endl;
		return -1;
	}

	// 开启深度测试
	// 深度テストを有効化
	glEnable(GL_DEPTH_TEST);

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

		//camera
		ImGui::Text("CameraX %f | CameraY %f | CameraZ %f | CameraPitch %f | CameraYaw %f | CameraFov %f",
			myCam.camPos.x, myCam.camPos.y, myCam.camPos.z, myCam.pitchValue, myCam.yawValue, myCam.fov);

		// clear color
		static vec3 bkgColor = vec3(0.0f, 0.0f, 0.0f);
		ImGui::ColorEdit3("background", (float*)&bkgColor);
		glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// direction light
		static vec3 dirLight_direction = vec3(-1.0f, -1.0f, -1.0f);
		static vec3 dirLight_ambient = vec3(0.2f);
		static vec3 dirLight_diffuse = vec3(0.8f);
		static vec3 dirLight_specular = vec3(1.0f);
		ImGui::ColorEdit3("dirLight direction", (float*)&dirLight_direction);
		ImGui::ColorEdit3("dirLight ambient", (float*)&dirLight_ambient);
		ImGui::ColorEdit3("dirLight diffuse", (float*)&dirLight_diffuse);
		ImGui::ColorEdit3("dirLight specular", (float*)&dirLight_specular);

		// point light
		static float posValue = 0.0f;
		static vec3 pointLight_ambient = vec3(0.2f);
		static vec3 pointLight_diffuse = vec3(0.8f);
		static vec3 pointLight_specular = vec3(1.0f);
		float constant = 1.0f;
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
				cout << "Light Fade Distance Error!" << endl;
				break;
			}
				
		}

		// spotLight
		static vec3 spotLight_ambient = vec3(0.2f);
		static vec3 spotLight_diffuse = vec3(0.8f);
		static vec3 spotLight_specular = vec3(1.0f);
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

		//激活myShader程序 这里涉及两个shader程序的切换，所以每次loop里都要在对应的位置调用，不能只在开始调用一次
		// 2つのシェーダープログラムの切り替えが必要なため、ループ毎に対応位置で呼び出すこと（初期呼び出しのみ不適）
		myShader.Use();

		// 设置纹理单元 任何uniform设置操作一定要放到《对应的shader》有效之后！  --》不同的shader切换运行，另一个shader会关掉，写的数据会丢失数据
        //也就是说启动了shader1之后又启动了shader2，之前在shader1设置的就无效了！这种情况只能放到渲染循环里，不能放循环外面
		// テクスチャユニットの設定：ユニフォーム変数の操作は必ず《対応するシェーダー》有効中に行う！  
		// → 別のシェーダーに切り替えると設定値が失われる  
		// 例: shader1起動後にshader2を起動 → shader1の設定は無効化  
		// 解決策: レンダリングループ内で対応するシェーダー有効中で設定（ループ外では不可）  
		myShader.SetInt("material.diffuse", 0);
		myShader.SetInt("material.specular", 1);

		myShader.SetVec3("uni_viewPos", myCam.camPos); 

		myShader.SetVec3("dirLight.direction", vec3(-1.0f, -1.0f, -1.0f));
		myShader.SetVec3("dirLight.ambient", dirLight_ambient);
		myShader.SetVec3("dirLight.diffuse", dirLight_diffuse);
		myShader.SetVec3("dirLight.specular", dirLight_specular);
		myShader.SetVec3("spotLight.lightPos", myCam.camPos);
		myShader.SetVec3("spotLight.direction", myCam.camFront);
		myShader.SetVec3("spotLight.ambient", spotLight_ambient);
		myShader.SetVec3("spotLight.diffuse", spotLight_diffuse);
		myShader.SetVec3("spotLight.specular", spotLight_specular);
		myShader.SetFloat("spotLight.innerCos", cos(radians(spotLight_innerCos)));
		myShader.SetFloat("spotLight.outerCos", cos(radians(spotLight_outerCos)));

		for (int i = 0; i < 4; i++)
		{
			std::stringstream ss;
			ss << "pointLight[" << i << "].";
			std::string prefix = ss.str();

			vec3 lightPos = vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));
			myShader.SetVec3(prefix + "lightPos", lightPos);
			myShader.SetVec3(prefix + "ambient", pointLight_ambient);
			myShader.SetVec3(prefix + "diffuse", pointLight_diffuse);
			myShader.SetVec3(prefix + "specular", pointLight_specular);
			myShader.SetFloat(prefix + "constant", 1.0f);
			myShader.SetFloat(prefix + "linear", linear);
			myShader.SetFloat(prefix + "quadratic", quadratic);
		}

		myShader.SetInt("material.shininess", material_shininess);

		glBindVertexArray(VAO);

		// 绑定显存,就可以进行独写操作，但是要读独写两块显存的时候，没办法同时绑定同一个GL_TEXTURE_2D，只能用纹理单元来区分
		// VRAMバインディングにより読み書き可能  
		// 同一GL_TEXTURE_2Dへの同時バインド不可 → テクスチャユニットで分離  
		glActiveTexture(GL_TEXTURE0);
		//片段着色器会根据GL_TEXTURE0读取texture_diffuse的贴图数据
		// フラグメントシェーダーがGL_TEXTURE0からtexture_diffuseをサンプリング  
		glBindTexture(GL_TEXTURE_2D, texture_diffuse);  
		glActiveTexture(GL_TEXTURE1);
		//片段着色器会根据GL_TEXTURE1读取texture_specular的贴图数据
		//フラグメントシェーダーがGL_TEXTURE1からtexture_specularをサンプリング  
		glBindTexture(GL_TEXTURE_2D, texture_specular); 
		
		// view矩阵 world -> view
		/* 変換行列生成処理 */
		// view行列（視点変換行列）
		mat4 view;
		myCam.setCamView();
		view = myCam.getCamView();
		myShader.SetMat4("uni_view", view);

		// 投影矩阵 view -> clip
		// projection行列（射影変換行列）
		mat4 projection;
		float fov = myCam.getCamFov();

		projection = perspective(radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f); // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
		myShader.SetMat4("uni_projection", projection);

		// model矩阵 local -> world
		// model行列（オブジェクト空間→ワールド空間変換）
		// 物体

		mat4 model = mat4(1.0f); // mat4初始化最好显示调用初始化为单位矩阵，因为新版本mat4 model可能是全0矩阵
	   // mat4の初期化は明示的に単位行列で行うべき　最新バージョンでは暗黙的にゼロ行列が生成される可能性あり
		myShader.SetMat4("uni_model", model);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		glBindVertexArray(0);

		lampShader.Use();

		glBindVertexArray(lightVAO);

		lampShader.SetMat4("uni_view", view);
		lampShader.SetMat4("uni_projection", projection);
		lampShader.SetVec3("uni_lightColor", vec3(1.0f));

		for (int i = 0; i < 4; i++)
		{
			std::stringstream ss;
			ss << "pointLight[" << i << "].";
			std::string prefix = ss.str();

			vec3 lightPos = vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));

			mat4 model = mat4(1.0f);
			model = scale(model, vec3(0.5f));
			model = translate(model, lightPos);
			model = rotate(model, radians(45.0f + i * 10), vec3(1.0f, 1.0f, 0.0f));

			lampShader.SetMat4("uni_model", model);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}


		glBindVertexArray(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//缓冲区交换
		//バッファ交換 
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
	// 计算当前帧与上一帧的时间差，用于修正渲染快慢对相机移动速度的影响
	// レンダリング速度の変動がカメラ移動速度に与える影響を補正するため、現在フレームと前フレームの時間差（デルタタイム）を算出
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

bool LoadTexture(const string&& filePath, GLuint& texture)
{
	// 申请显存空间并绑定GL_TEXTURE_2D对象
	// VRAM領域確保し、GL_TEXTURE_2Dオブジェクトをバインド
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// 设置GL_TEXTURE_2D的环绕，过滤方式
	// GL_TEXTURE_2Dのラップモードとフィルタリング設定
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载贴图，转换为数据
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
		// 贴图数据传入显存
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

	return true;
}

void loadModel(string path)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
		return;
	}
	//directory = path.substr(0, path.find_last_of('/'));

	//processNode(scene->mRootNode, scene);
}