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
#include "Scene.h"

bool InitOpenGL();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void GetImguiValue();
void SetUniformToShader(Shader& shader);
void CreateFrameBuffer(GLuint& fbo, GLuint& tbo, GLuint& rbo);
void CreateFrameBuffer_MSAA(GLuint& fbo, GLuint& tbo, GLuint& rbo);
void SetUniformBuffer();

Camera myCam(vec3(0.635f, 2.605f, -1.247f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
GLFWwindow* window = NULL;

// 原シーン バッファ
GLuint fbo1 = 0; 
GLuint tbo1 = 0;
GLuint rbo1 = 0;

// バックミラー バッファ
GLuint fbo2 = 0; 
GLuint tbo2 = 0;
GLuint rbo2 = 0; 

// 中间バッファ
GLuint fbo3 = 0;
GLuint tbo3 = 0;
GLuint rbo3 = 0;

// Uniform　バッファ
GLuint ubo = 0;

int main()
{
	if (!InitOpenGL())
	{
		cout << "Failed to initialize." << endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);               // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
	
	Scene scene;
	scene.CreateScene(&myCam);

	// 原シーン
	CreateFrameBuffer_MSAA(fbo1, tbo1, rbo1);
	// バックミラー
	CreateFrameBuffer(fbo2, tbo2, rbo2);
	// 中间バッファ
	CreateFrameBuffer(fbo3, tbo3, rbo3);

	// Uniform　バッファ
	// 参照 Referrence/uniform buffer binding.png
	// 获取shader uniform block index，在opengl 420以上版本可以直接用layout(std140, binding = XXX)在shader直接指定 
	// shader uniform block indexを取得。OpenGL 4.20以降ではシェーダー内でlayout(std140, binding=XXX)を直接指定可能
	GLuint ubi_Lighting = glGetUniformBlockIndex(scene.lightShader.ID, "Matrix");
	GLuint ubi_Cubemap = glGetUniformBlockIndex(scene.cubemapShader.ID, "Matrix");
	GLuint ubi_Refract  = glGetUniformBlockIndex(scene.refractShader.ID, "Matrix");
	GLuint ubi_Reflect  = glGetUniformBlockIndex(scene.reflectShader.ID, "Matrix");

	// shader uniform block => binding point
	glUniformBlockBinding(scene.lightShader.ID, ubi_Lighting, 0);
	glUniformBlockBinding(scene.cubemapShader.ID, ubi_Cubemap, 0);
	glUniformBlockBinding(scene.refractShader.ID, ubi_Refract, 0);
	glUniformBlockBinding(scene.reflectShader.ID, ubi_Reflect, 0);

	// uboを作成
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_STATIC_DRAW); // 只有4->16的情况才要考虑内存对齐。NULL表示只分配内存，不写入数据。
	
	// ubo => binding point
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	GLuint t_dummy = 0;

	const vector<Texture> dummyTexture =
	{
		{t_dummy, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

	//　レンダリングループ
	while (!glfwWindowShouldClose(window))
	{
		//入力
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

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		if (bMSAA)
			glEnable(GL_MULTISAMPLE); // 很多opengl驱动不需要显式地调用，但是还是调用一下保险一点
									  // 多くのOpenGLドライバでは明示的な有効化不要だが、念のため呼び出し
		else 
			glDisable(GL_MULTISAMPLE);

		curTime = glfwGetTime();
		deltaTime = curTime - preTime;
		preTime = curTime;

		/********************** 先用自定义帧缓冲进行离屏渲染 **********************/
		/********************** カスタムフレームバッファによるオフスクリーンレンダリング **********************/
		// 绑定到自定义帧缓冲，关闭对默认帧缓冲的读写
		// カスタムフレームバッファにバインド（デフォルトフレームバッファへの読み書きを無効化）
		
		// 原シーン
		SetUniformBuffer();
		SetUniformToShader(scene.lightShader);
		SetUniformToShader(scene.screenShader);
		SetUniformToShader(scene.cubemapShader);
		SetUniformToShader(scene.reflectShader);
		SetUniformToShader(scene.refractShader);
		SetUniformToShader(scene.normalShader);
		SetUniformToShader(scene.lightInstShader);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
		scene.DrawScene();

		// 用中间fbo的方式实现，实际上中间FBO就是一个只带1个采样点的普通帧缓冲。用Blit操作把MSAA FBO复制进去，然后就可以用中间FBO的TBO来后期处理了。
		// 缺点是，如果在此基础上进行后处理去计算颜色，是以1个采样点的纹理为基础来计算的，可能会重新导致锯齿
		// 中間FBOを用いた実装方式。中間FBOは1サンプル点のみの通常フレームバッファ。Blit操作でMSAA FBOからコピー後、中間FBOのTBOで後処理可能
		// 欠点：この状態で色計算を行う後処理を適用すると、1サンプル点ベースの計算になるため再びジャギー発生の可能性あり
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo3);

		// MSAA纹理本身其实也可以直接传到着色器进行采样，因为MSAA纹理格式和普通纹理不一样，所以不能像普通纹理对象直接用。
		// 可以用sampler2DMS的方式传入，可以获取到每个采样点，主要用于自定义抗锯齿算法
		// 只是单纯渲染场景的话，用glBlitFramebuffer就够了
		// MSAAテクスチャ自体はシェーダーに直接渡せるが、通常テクスチャとフォーマットが異なるため通常のsampler2Dではサンプリング不可
		// sampler2DMS型を使用すれば各サンプル点取得可能（カスタムアンチエイリアスアルゴリズム用）
		// シーン描画のみの場合はglBlitFramebufferで十分
		glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// バックミラー
		myCam.yawValue += 180.0;
		SetUniformBuffer();
		SetUniformToShader(scene.lightShader);
		SetUniformToShader(scene.screenShader);
		SetUniformToShader(scene.cubemapShader);
		SetUniformToShader(scene.reflectShader);
		SetUniformToShader(scene.refractShader);
		SetUniformToShader(scene.normalShader);
		SetUniformToShader(scene.lightInstShader);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo2); 
		scene.DrawScene();
		myCam.yawValue -= 180.0;
		SetUniformBuffer();
		SetUniformToShader(scene.lightShader);
		SetUniformToShader(scene.screenShader);
		SetUniformToShader(scene.cubemapShader);
		SetUniformToShader(scene.reflectShader);
		SetUniformToShader(scene.refractShader);
		SetUniformToShader(scene.normalShader);
		SetUniformToShader(scene.lightInstShader);

		/********************** 绑定回默认帧缓冲 **********************/
		/********************** デフォルトフレームバッファへの再バインド **********************/
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 原シーン
		const vector<Texture> screenTexture =
		{
			{tbo3, "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		scene.screen.SetTextures(screenTexture);
		scene.screen.DrawMesh(scene.screenShader, GL_TRIANGLES);

		// バックミラー
		const vector<Texture> mirrorTexture =
		{
			{tbo2, "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		scene.mirror.SetTextures(mirrorTexture);
		scene.mirror.DrawMesh(scene.screenShader, GL_TRIANGLES);

		// imgui在默认缓冲中绘制，因为我不想imgui也有后期处理效果
		// ImGUIのレンダリングはデフォルトフレームバッファで行う（ポストプロセス効果を適用しないため）
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	scene.DeleteScene();
	glDeleteFramebuffers(1, &fbo1);
	glDeleteFramebuffers(1, &tbo1);
	glDeleteFramebuffers(1, &rbo1);
	glDeleteFramebuffers(1, &fbo2);
	glDeleteFramebuffers(1, &tbo2);
	glDeleteFramebuffers(1, &rbo2);
	glDeleteBuffers(1, &ubo);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	// bugfix 窗口大小变化后，纹理缓冲的大小也要相应变化
	// 删除已存在的缓冲
	// バグ修正: ウィンドウサイズ変更時にテクスチャバッファのリサイズが必要
	// 既存バッファの削除
	glDeleteFramebuffers(1, &fbo1);
	glDeleteFramebuffers(1, &tbo1);
	glDeleteFramebuffers(1, &rbo1);
	glDeleteFramebuffers(1, &fbo2);
	glDeleteFramebuffers(1, &tbo2);
	glDeleteFramebuffers(1, &rbo2);
	// 原シーン
	CreateFrameBuffer(fbo1, tbo1, rbo1);
	// バックミラー
	CreateFrameBuffer(fbo2, tbo2, rbo2);

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

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		return;
	}

	/* FOV */
	if (myCam.fov >= 1.0f && myCam.fov <= 95.0f)
		myCam.fov -= yoffset;
	if (myCam.fov <= 1.0f)
		myCam.fov = 1.0f;
	if (myCam.fov >= 95.0f)
		myCam.fov = 95.0f;
}

void GetImguiValue()
{
	if (ImGui::TreeNodeEx("Camera"))
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

	if (ImGui::TreeNodeEx("Lighting"))
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

	if (ImGui::TreeNodeEx("Test and Blend"))
	{
		ImGui::Checkbox("Blending", &bBlending);
		ImGui::Checkbox("Face Culling", &bFaceCulling);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("PostProcess"))
	{
		const char* itemArray[] = {"Default", "Sharpen", "Edge Detection", "Blur"};
		ImGui::Combo("PostProcess Type", &postProcessType, itemArray, IM_ARRAYSIZE(itemArray));

		ImGui::SliderFloat("PostProcess sample offset", &sampleOffsetBase, 1.0f, 3000.0f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Skybox", &bSkyBox);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Advanced GLSL"))
	{
		ImGui::SliderFloat("Point Size", &pointSize, 0.0f, 50.0f);
		ImGui::Checkbox("Split Screen", &bSplitScreen);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Geometry Shader"))
	{
		ImGui::Checkbox("Geometry Shader Test", &bGMTest);
		ImGui::SliderFloat("Explode Magnitude", &explodeMag, 0.0f, 5.0f);
		ImGui::SliderFloat("Normal Length", &normalLen, 0.0f, 0.8f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Instance", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Instance Test", &bInstanceTest);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("AntiAliasing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("MSAA", &bMSAA);
		ImGui::TreePop();
	}
}

void SetUniformToShader(Shader& shader)
{
	// 这里涉及两个shader程序的切换，所以每次loop里都要在对应的位置调用，不能只在开始调用一次
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
	shader.SetFloat("window_width", windowWidth);
	shader.SetFloat("window_height", windowHeight);
	shader.SetInt("split_flag", (int)bSplitScreen);
	shader.SetFloat("magnitude", explodeMag);
	shader.SetFloat("normal_len", normalLen);

	// ShaderLightingInstance 
	// 因为model矩阵变换是基于单位矩阵进行的，想要在已经变换后的model矩阵的基础上，再进行model矩阵变换有点困难
	mat4 model = mat4(1.0f);
	//float angle = (float)i / (float)ROCK_NUM * 360.0f;
	//model = translate(model, vec3(0.0f, 0.0f, 0.0f));

	//float _scale = (rand() % 20) / 100.0f + 0.05;
	//model = scale(model, vec3(0.2));

	float rotAngle = float(curTime * ROTATE_SPEED_ROCK);
	model = rotate(model, rotAngle, vec3(0.0f, 1.0f, 0.0f));

	shader.SetMat4("extra_model", model);

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
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

void CreateFrameBuffer_MSAA(GLuint& fbo, GLuint& tbo, GLuint& rbo)
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// フレームバッファオブジェクト（FBO）の生成（color stencil depthを含む。デフォルトバッファも同様だが、独自バッファを作成することで特殊効果が実現可能）
	// 只有默认缓冲才能输出图像，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	// デフォルトバッファのみが画面出力可能。自作バッファは画面表示されないため、オフスクリーンレンダリングに活用
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 生成MSAA纹理附件 对应color缓冲
	// MSAAテクスチャアタッチメント生成（カラーバッファ対応）
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbo);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLE_NUM, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, true);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// MSAA纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT0附件 附加到 帧缓冲对象
	// TBOをGL_COLOR_ATTACHMENT0としてFBOにアタッチ
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tbo, 0);

	// 生成渲染缓冲对象 对应stencil，depth缓冲
	// レンダーバッファオブジェクト(RBO)生成（ステンシル・深度バッファ対応）
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// stencil，depth和color缓冲应该都要对应MSAA，如果color是对应了MSAA 而stencil，depth没有对应MSAA，stencil depth和color是不匹配的，导致错误
	// ステンシル・深度・カラーバッファは全てマルチサンプル対応が必要。カラーのみMSAA対応でステンシル/深度が非対応の場合、バッファ間の不一致が生じて描画エラーの原因となる
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLE_NUM, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); 
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

void SetUniformBuffer()
{
	// view矩阵 world -> view
	// view行列（視点変換行列）
	mat4 view;
	myCam.setCamView();
	view = myCam.getCamView();
	
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);

	//　调用glBufferSubData填充数据之前要确保已经调用glBufferData分配了内存
	// glBufferSubDataでデータを書き込む前に、glBufferDataによるメモリ確保が完了していることを確認
	// 
	// std140 layout 对齐偏移量必须是16的倍数 
	// std140レイアウトにおけるアライメントオフセットは16の倍数でなければならない
	// 
	// 参照 Referrence/std140 layout.png
	// 
	// 用glBufferSubData写数据的时候要考虑std140 layout
	// glBufferSubDataでデータを書き込む際はstd140レイアウトを考慮する必要がある
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(view)); 

	// 投影矩阵 view -> clip
	// projection行列（射影変換行列）
	mat4 projection;
	float fov = myCam.getCamFov();

	// 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
	// 以前は(float)(WINDOW_WIDTH / WINDOW_HEIGHT)と記述していたため、整数除算で精度が失われ結果が1になっていた
	projection = perspective(radians(fov), (float)windowWidth / (float)windowHeight, myCam.camNear, myCam.camFar); // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(projection));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool InitOpenGL()
{
	// 初期化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLE_NUM); // 和窗口绑定的采样点，显然，窗口对应的是默认帧缓冲

	// ウィンドウを作成する
	window = glfwCreateWindow(windowWidth, windowHeight, "koalahjf", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	// マウスキャプチャ
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialze GLAD." << endl;
		glfwTerminate();
		return false;
	}
	glViewport(0, 0, windowWidth, windowHeight);

	return true;
}
