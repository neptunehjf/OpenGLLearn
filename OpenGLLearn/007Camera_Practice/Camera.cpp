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

using namespace std;
using namespace glm;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera myCam(vec3(1.0f, 2.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialze GLAD." << endl;
		glfwTerminate();
		return -1;
	}

	// シェーダープログラムを作成
	// 
	// 默认vs的工作路径是在ProjectDir下，所以使用相对路径的话，应该把shader文件也放到ProjectDir
	// デフォルトではVSの作業パスはプロジェクトディレクトリ下に設定されているため、相対パスを使用する場合はシェーダーファイルもプロジェクトディレクトリに配置する必要があります

	Shader myShader("shader.vs", "shader.fs"); 

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

	// 存储下标数据到EBO
	// インデックスデータをEBOに格納
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	// 翻转y轴，使图片坐标和opengl坐标一致
    // Y軸を反転して画像座標とOpenGL座標を一致させる
	stbi_set_flip_vertically_on_load(true);

	// 申请显存空间并绑定GL_TEXTURE_2D对象
	// VRAM領域確保し、GL_TEXTURE_2Dオブジェクトをバインド
	GLuint texture0 = 0;
	glGenTextures(1, &texture0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	// 设置GL_TEXTURE_2D的环绕，过滤方式
	// GL_TEXTURE_2Dのラップモードとフィルタリング設定
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载贴图，转换为数据
	// テクスチャ読み込み、データに変換
	int width, height, channel;
	unsigned char *data = stbi_load("stone_wall.jpg", &width, &height, &channel, 0);
	if (data)
	{
		// 贴图数据传入显存
    	/* テクスチャデータをVRAMに転送する */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
	{
		cout << "Failed to load texture0！" << endl;
		return -1;
	}
	// 数据已经传给显存了，删除内存中的数据
	// メモリ上のデータ削除（VRAMに転送完了後）
	stbi_image_free(data);

	// 申请显存空间并绑定GL_TEXTURE_2D对象
	// VRAM領域確保し、GL_TEXTURE_2Dオブジェクトをバインド
	GLuint texture1 = 0;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	// 设置GL_TEXTURE_2D的环绕，过滤方式
	// GL_TEXTURE_2Dのラップモードとフィルタリング設定
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// 加载贴图，转换为数据
	// テクスチャ読み込み、データに変換
	data = stbi_load("awesomeface.png", &width, &height, &channel, 0);
	if (data)
	{
		// 贴图数据传入显存
		// テクスチャデータをVRAMに転送
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture1！" << endl;
		return -1;
	}
	// 数据已经传给显存了，删除内存中的数据
	// メモリ上のデータ削除（VRAMに転送完了後）
	stbi_image_free(data);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, COLOR_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, STRIDE_SIZE * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// シェーダーを実行
	if (myShader.Use() == false)
	{
		cout << "Shader program invalid!" << endl;
		return -1;
	}

	// 设置sampler变量对应纹理单元 对uniform的操作必须在shader激活之后
	// サンプラー変数とテクスチャユニットを紐付け  uniform操作はシェーダー実行の後に行うこと
	myShader.SetInt("uni_texture0", 0);
	myShader.SetInt("uni_texture1", 1);

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

		//渲染之前清空窗口
		//レンダリング前にウィンドウをクリアする
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		// 绘制
		glActiveTexture(GL_TEXTURE0);           
		glBindTexture(GL_TEXTURE_2D, texture0); // 在这里绑定会把纹理读到 纹理单元0里
		                                        // このバインド操作でtexture0をテクスチャユニット0に割り当て

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1); // 在这里绑定会把纹理读到 纹理单元1里
												// このバインド操作でtexture1をテクスチャユニット1に割り当て

		// 绑定VAO上下文以读取顶点数据
		// 頂点データを読み取るためにVAOをバインド
		glBindVertexArray(VAO);

		/* 生成变换矩阵 */
		// view矩阵
		/* 変換行列生成処理 */
		// view行列（視点変換行列）
		mat4 view;
		myCam.setCamView();
		view = myCam.getCamView();
		myShader.SetMat4("uni_view", view);
		// 投影矩阵
		// projection行列（射影変換行列）
		mat4 projection;
		float fov = myCam.getCamFov();
		projection = perspective(radians(fov), (float)(WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f, 100.0f);
		myShader.SetMat4("uni_projection", projection);

		for (int i = 0; i < 10; i++)
		{
			// model矩阵
			// model行列（オブジェクト空間→ワールド空間変換）
			mat4 model;
			model = translate(model, cubePositions[i]);
			model = rotate(model, (i + 1) * radians(50.0f), vec3(0.5f, 1.0f, 0.0f));
			myShader.SetMat4("uni_model", model);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//缓冲区交换
		//バッファ交換 
		glfwSwapBuffers(window);

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

	//cout << "#" << myCam.camPos.x << "#" << myCam.camPos.y << "#" << myCam.camPos.z << endl;
}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
	if (myCam.isFirst)
	{
		myCam.lastX = posX;
		myCam.lastY = posY;
		myCam.isFirst = false;
	}

	float offsetX = posX - myCam.lastX;
	float offsetY = myCam.lastY - posY;
	myCam.lastX = posX;
	myCam.lastY = posY;

	myCam.yawValue += offsetX;
	myCam.pitchValue += offsetY;
	float speed = 1.0f;
	myCam.yawValue *= speed;
	myCam.pitchValue *= speed;

	if (myCam.pitchValue > 89.0f)
		myCam.pitchValue = 89.0f;
	if (myCam.pitchValue < -89.0f)
		myCam.pitchValue = -89.0f;

	
	// 相机旋转
	// カメラ回転
	// 对于yaw，设camera坐标系的+Z从+X开始逆时针旋转计算
	// ヨー角計算時、カメラ座標系の+Z方向は+X軸を起点とする反時計回り回転として定義されます
	// 参照Referrence/camera rotate.jpg Referrence/Euler Angle.png
	vec3 front;
	front.x = cos(radians(myCam.yawValue)) * cos(radians(myCam.pitchValue)); // 因为视角默认朝向X轴正方向，所以应该用与X轴正方向的角度计算偏移
	front.y = sin(radians(myCam.pitchValue));
	front.z = sin(radians(myCam.yawValue)) * cos(radians(myCam.pitchValue)); // 因为视角默认朝向X轴正方向，所以应该用与X轴正方向的角度计算偏移
	
	myCam.camFront = normalize(front);
	//cout << "@@" << myCam.camFront.x << "@@" << myCam.camFront.y << "@@" << myCam.camFront.z << endl;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (myCam.fov >= 1.0f && myCam.fov <= 95.0f)
		myCam.fov -= yoffset;
	if (myCam.fov <= 1.0f)
		myCam.fov = 1.0f;
	if (myCam.fov >= 95.0f)
		myCam.fov = 95.0f;
}