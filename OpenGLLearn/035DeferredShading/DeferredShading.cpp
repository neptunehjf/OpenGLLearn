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
void CreateFrameBuffer(GLuint& fbo, GLuint* tbo, GLuint& rbo, GLuint tbo_num);
void CreateFrameBuffer_MSAA(GLuint& fbo, GLuint* tbo, GLuint& rbo, GLuint tbo_num);
void CreateFrameBuffer_Depthmap(GLuint& fbo, GLuint& tbo);
void CreateFrameBuffer_DepthCubemap(GLuint& fbo, GLuint& tbo);
void CreateFrameBuffer_pingpong();
void CreateFrameBuffer_G();
void SetUniformBuffer();
void DrawDepthMap();
void DrawDepthCubemap(vec3 lightPos);
void SetAllUniformValues();
void DrawScreen();


Scene scene;
Camera myCam(vec3(2.158f, 3.304f, -0.636f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
GLFWwindow* window = NULL;

// 原场景缓冲
GLuint fbo_origin = 0; // 自定义帧缓冲对象
GLuint tbo_origin[2] = {}; // 纹理缓冲对象（附件）
GLuint rbo_origin = 0; // 渲染缓冲对象（附件）

// 后视镜缓冲
GLuint fbo_mirror = 0; // 自定义帧缓冲对象
GLuint tbo_mirror[1] = {}; // 纹理缓冲对象（附件）
GLuint rbo_mirror = 0; // 渲染缓冲对象（附件）

// 中间缓冲
GLuint fbo_middle = 0; // 自定义帧缓冲对象
GLuint tbo_middle[2] = {}; // 纹理缓冲对象（附件）
GLuint rbo_middle = 0; // 渲染缓冲对象（附件）

// depthmap缓冲
GLuint fbo_depthmap = 0; // 自定义帧缓冲对象
GLuint tbo_depthmap = 0; // 纹理缓冲对象（附件）

// depthCubemap缓冲
GLuint fbo_depthCubemap = 0; // 自定义帧缓冲对象
GLuint tbo_depthCubemap = 0; // 纹理缓冲对象（附件）

// Uniform缓冲
GLuint ubo = 0;

// pingpong缓冲
GLuint fbo_pingpong[2] = {}; // 自定义帧缓冲对象
GLuint tbo_pingpong[2] = {}; // 纹理缓冲对象（附件）

// G缓冲
GLuint fbo_G = 0;             // 自定义帧缓冲对象
GLuint tbo_G_position = 0;    // 纹理缓冲对象（附件） 存储位置信息
GLuint tbo_G_normal = 0;      // 纹理缓冲对象（附件） 存储法线信息
GLuint tbo_G_abdspec = 0;     // 纹理缓冲对象（附件） 存储反照率和高光信息
GLuint rbo_G = 0;             // 渲染缓冲对象（附件）

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
	
	scene.CreateShader();
	//scene.CreateScene(&myCam);

	// 原场景缓冲
	CreateFrameBuffer_MSAA(fbo_origin, tbo_origin, rbo_origin, 2);
	// 后视镜缓冲
	CreateFrameBuffer(fbo_mirror, tbo_mirror, rbo_mirror, 1);
	// 中间缓冲
	CreateFrameBuffer(fbo_middle, tbo_middle, rbo_middle, 2);
	// depthmap缓冲
	CreateFrameBuffer_Depthmap(fbo_depthmap, tbo_depthmap);
	// depthCubemap缓冲
	CreateFrameBuffer_DepthCubemap(fbo_depthCubemap, tbo_depthCubemap);
	// G缓冲
	CreateFrameBuffer_G();

	// Uniform缓冲
	// 
	// 根据UniformBlockIndex绑定各shader的uniformblock到binding point，在opengl 420以上版本可以直接用layout(std140, binding = XXX)在shader直接指定 
	GLuint ubi_Lighting = glGetUniformBlockIndex(scene.lightShader.ID, "Matrix");
	GLuint ubi_Cubemap = glGetUniformBlockIndex(scene.cubemapShader.ID, "Matrix");
	GLuint ubi_Refract  = glGetUniformBlockIndex(scene.refractShader.ID, "Matrix");
	GLuint ubi_Reflect  = glGetUniformBlockIndex(scene.reflectShader.ID, "Matrix");

	glUniformBlockBinding(scene.lightShader.ID, ubi_Lighting, 0);
	glUniformBlockBinding(scene.cubemapShader.ID, ubi_Cubemap, 0);
	glUniformBlockBinding(scene.refractShader.ID, ubi_Refract, 0);
	glUniformBlockBinding(scene.reflectShader.ID, ubi_Reflect, 0);

	// 创建Uniform缓冲区，并绑定到对应的binding point
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_STATIC_DRAW); // 只有4->16的情况才要考虑内存对齐。NULL表示只分配内存，不写入数据。
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	bool bLastGamma = false; 
	bool bLastNM = false;
	bool bLastHDR = false;

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		if (bGammaCorrection)
		{
			glEnable(GL_FRAMEBUFFER_SRGB); //颜色写到帧缓冲之前会被gamma校正
			if (!bLastGamma)
				scene.CreateScene(&myCam);
			bLastGamma = true;
		}
		else if (!bGammaCorrection)
		{
			glDisable(GL_FRAMEBUFFER_SRGB); 
			if (bLastGamma)
				scene.CreateScene(&myCam);
			bLastGamma = false;
		}

		if (!bLastNM && bEnableNormalMap)
		{
			scene.UpdateNMVertices();
			bLastNM = true;
		}
		if (bLastNM && !bEnableNormalMap)
		{
			scene.UpdateNMVertices();
			bLastNM = false;
		}

		if (bHDR)
		{
			if (!bLastHDR)
				scene.CreateScene(&myCam);
			bLastHDR = true;
		}
		else if (!bHDR)
		{
			if (bLastHDR)
				scene.CreateScene(&myCam);
			bLastHDR = false;
		}

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

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		if (bMSAA)
			glEnable(GL_MULTISAMPLE); // 很多opengl驱动不需要显式地调用，但是还是调用一下保险一点
		else 
			glDisable(GL_MULTISAMPLE);

		curTime = glfwGetTime();
		deltaTime = curTime - preTime;
		preTime = curTime;

		/********************** 先用自定义帧缓冲进行离屏渲染 绑定到自定义帧缓冲，默认帧缓冲不再起作用 **********************/
		
		if (bShadow)
		{
			DrawDepthMap();
			DrawDepthCubemap(lampWithShadowPos);
		}

		// 原场景
		glViewport(0, 0, windowWidth, windowHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo_origin);

		scene.DrawScene();

		// 用中间fbo的方式实现，实际上中间FBO就是一个只带1个采样点的普通帧缓冲。用Blit操作把MSAA FBO复制进去，然后就可以用中间FBO的TBO来后期处理了。
		// 缺点是，如果在此基础上进行后处理去计算颜色，是以1个采样点的纹理为基础来计算的，可能会重新导致锯齿
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_origin);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_middle);

		// MSAA纹理本身其实也可以直接传到着色器进行采样，因为MSAA纹理格式和普通纹理不一样，所以不能像普通纹理对象直接用。
		// 可以用sampler2DMS的方式传入，可以获取到每个采样点，主要用于自定义抗锯齿算法
		// 只是单纯渲染场景的话，用glBlitFramebuffer就够了
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer(0);
		glDrawBuffer(0);

		// 后视镜场景
		myCam.yawValue += 180.0;
		SetAllUniformValues();
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_mirror); 
		scene.DrawScene();
		myCam.yawValue -= 180.0;
		SetAllUniformValues();
		myCam.yawValue = myCam.yawValue - ((int)myCam.yawValue / 360) * 360;

		/********************** 默认帧缓冲输出前面绘制时写入 **********************/
		DrawScreen();

		glDisable(GL_FRAMEBUFFER_SRGB); //imgui界面不需要gamma校正
		// imgui在默认缓冲中绘制，因为我不想imgui也有后期处理效果
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 缓冲区交换 轮询事件
		glfwSwapBuffers(window);
	}

	// 资源清理
	scene.DeleteScene();
	glDeleteFramebuffers(1, &fbo_origin);
	glDeleteFramebuffers(1, &fbo_mirror);
	glDeleteFramebuffers(1, &fbo_middle);

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
	glDeleteFramebuffers(1, &fbo_origin);
	glDeleteFramebuffers(1, &fbo_mirror);
	glDeleteFramebuffers(1, &fbo_middle);

	// 原场景缓冲
	CreateFrameBuffer_MSAA(fbo_origin, tbo_origin, rbo_origin, 2);
	// 后视镜缓冲
	CreateFrameBuffer(fbo_mirror, tbo_mirror, rbo_mirror, 1);
	// 中间缓冲
	CreateFrameBuffer(fbo_middle, tbo_middle, rbo_middle, 2);

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
	// 鼠标右键不按就不处理，因为鼠标要用来点Imgui
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		return;
	}

	/* 镜头缩放 */
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
		myCam.camFar = imgui_camFar; // 如果用其他变量接收imgui变量，必须保证两者初始值一致，因为imgui收起的状态是不传值的。

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Lighting"))
	{
		// 光照模型
		const char* LightModels[] = { "Phong", "Blinn-Phong" };
		ImGui::Combo("Light Model", &iLightModel, LightModels, IM_ARRAYSIZE(LightModels));

		// clear color
		ImGui::ColorEdit3("background", (float*)&bkgColor);

		// direction light
		ImGui::DragFloat3("dirLight direction", (float*)&dirLight_direction);
		ImGui::ColorEdit3("dirLight ambient", (float*)&dirLight_ambient);
		ImGui::ColorEdit3("dirLight diffuse", (float*)&dirLight_diffuse);
		ImGui::ColorEdit3("dirLight specular", (float*)&dirLight_specular);

		// point light
		const char* itemArray[] = { "50", "100", "200", "600" };
		const char* atteFormulas[] = { "default", "linear", "quadratic" };
		ImGui::DragFloat3("PointLightWithShadow Position", (float*)&lampWithShadowPos);
		ImGui::ColorEdit3("pointLight ambient", (float*)&pointLight_ambient);
		ImGui::ColorEdit3("pointLight diffuse", (float*)&pointLight_diffuse);
		ImGui::ColorEdit3("pointLight specular", (float*)&pointLight_specular);
		ImGui::Combo("Attenuation Formula", &iAtteFormula, atteFormulas, IM_ARRAYSIZE(atteFormulas));

		if (iAtteFormula == 0)
		{
			ImGui::Combo("Light Fade Distance", &item, itemArray, IM_ARRAYSIZE(itemArray));
		}

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

	if (ImGui::TreeNodeEx("Skybox"))
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
		ImGui::SliderFloat("Explode Magnitude", &explodeMag, 0.0f, 5.0f);
		ImGui::SliderFloat("Normal Length", &normalLen, 0.0f, 0.8f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Instance"))
	{
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("AntiAliasing"))
	{
		ImGui::Checkbox("MSAA", &bMSAA);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Gamma Correction"))
	{
		ImGui::Checkbox("Gamma Correction", &bGammaCorrection);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Shadow Mapping"))
	{
		ImGui::Checkbox("Enable Shadow", &bShadow);
		ImGui::Checkbox("Depthmap Debug", &bDisDepthmap);
		ImGui::Checkbox("Depth Cubemap Debug", &bDepthCubemapDebug);

		ImGui::SliderFloat("Directional Shadow Bias", &fBiasDirShadow, 0.000f, 0.020f);
		ImGui::SliderFloat("Point Shadow Far Plane", &fFarPlanePt, 0.0f, 200.0f);
		ImGui::SliderFloat("Point Shadow Bias", &fBiasPtShadow, 0.000f, 1.0f);
		ImGui::Checkbox("Enable Front Face Culling", &bFrontFaceCulling); 

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Normal Mapping"))
	{
		ImGui::Checkbox("Enable Normal Mapping", &bEnableNormalMap);
		ImGui::Checkbox("Enable Parrallax Mapping", &bEnableParallaxMap);

		const char* aParaAlgo[] = { "Parallax Mapping", "Steep Parallax Mapping", "Parallax Occlusion Mapping" };
		ImGui::Combo("Parallax Sample Algorithm", &iParaAlgo, aParaAlgo, IM_ARRAYSIZE(aParaAlgo));

		ImGui::SliderFloat("Height Scale", &height_scale, 0.0f, 0.1f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Debug"))
	{
		ImGui::Checkbox("Enable Debug", &bDebug);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("HDR", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable HDR", &bHDR);
		const char* aAlgro[] = { "reinhard tone mapping", "exposure tone mapping" };
		ImGui::Combo("Tone Mapping Algorithm", &iHDRAlgro, aAlgro, IM_ARRAYSIZE(aAlgro));

		if (iHDRAlgro == 1)
			ImGui::SliderFloat("Exposure", &fExposure, 0.0f, 5.0f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable Bloom", &bBloom);

		ImGui::TreePop();
	}

	

}

void SetUniformToShader(Shader& shader)
{
	//激活lightShader程序 这里涉及两个shader程序的切换，所以每次loop里都要在对应的位置调用，不能只在开始调用一次
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
	shader.SetVec3("dirLight.direction", dirLight_direction);
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
	shader.SetInt("light_model", iLightModel);
	shader.SetInt("atte_formula", iAtteFormula);
	shader.SetBool("bShadow", bShadow);
	shader.SetFloat("fBiasDirShadow", fBiasDirShadow);
	shader.SetFloat("fBiasPtShadow", fBiasPtShadow);
	shader.SetFloat("height_scale", height_scale);
	shader.SetInt("iParaAlgo", iParaAlgo);
	shader.SetBool("bHDR", bHDR);
	shader.SetFloat("fExposure", fExposure);
	shader.SetInt("iHDRAlgro", iHDRAlgro);
	shader.SetBool("bBloom", bBloom);

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

		//vec3 lightPos = vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));
		shader.SetVec3(prefix + "lightPos", lampPos[i]);
		shader.SetVec3(prefix + "ambient", pointLight_ambient);
		shader.SetVec3(prefix + "diffuse", pointLight_diffuse);
		shader.SetVec3(prefix + "specular", pointLight_specular);
		shader.SetFloat(prefix + "constant", 1.0f);
		shader.SetFloat(prefix + "linear", linear);
		shader.SetFloat(prefix + "quadratic", quadratic);
	}

	shader.SetVec3("pointLight[4].lightPos", lampWithShadowPos); 
	shader.SetVec3("pointLight[4].ambient", pointLight_ambient);
	shader.SetVec3("pointLight[4].diffuse", pointLight_diffuse);
	shader.SetVec3("pointLight[4].specular", pointLight_specular);
	shader.SetFloat("pointLight[4].constant", 1.0f);
	shader.SetFloat("pointLight[4].linear", linear);
	shader.SetFloat("pointLight[4].quadratic", quadratic);
}

//创建自定义帧缓冲
void CreateFrameBuffer(GLuint& fbo, GLuint* tbo, GLuint& rbo, GLuint tbo_num)
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// 只有默认缓冲才能输出图像(因为和GLFW窗口绑定)，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// 生成纹理附件 对应color缓冲
	glGenTextures(tbo_num, tbo);

	for (uint i = 0; i < tbo_num; i++)
	{
		glBindTexture(GL_TEXTURE_2D, tbo[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// 纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT0附件 附加到 帧缓冲对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tbo[i], 0);	
	}

	// 生成渲染缓冲对象 对应stencil，depth缓冲
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 渲染缓冲对象 作为一个GL_DEPTH_STENCIL_ATTACHMENT附件 附加到 帧缓冲上
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	
	// 指定写入的颜色附件
	if (tbo_num == 2)
	{
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	}

	// 检查帧缓冲对象完整性
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//创建自定义帧缓冲 depthmap
void CreateFrameBuffer_Depthmap(GLuint& fbo, GLuint& tbo)
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// 只有默认缓冲才能输出图像(因为和GLFW窗口绑定)，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 生成纹理附件 对应Depth缓冲
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	// 深度图的数据类型应该是 GL_FLOAT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 纹理缓冲对象  作为一个GL_DEPTH_ATTACHMENT附件 附加到 帧缓冲对象
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tbo, 0);

	// 颜色缓冲的独写对象改为GL_NONE，也就是不会读写颜色缓冲
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// 检查帧缓冲对象完整性
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//创建自定义帧缓冲 depth cubemap
void CreateFrameBuffer_DepthCubemap(GLuint& fbo, GLuint& tbo)
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// 只有默认缓冲才能输出图像(因为和GLFW窗口绑定)，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 生成纹理附件 对应depth缓冲
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo);
	for (uint i = 0; i < 6; i++)
	{
		// 深度图的数据类型应该是 GL_FLOAT
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// 纹理缓冲对象  作为一个GL_DEPTH_ATTACHMENT附件 附加到 帧缓冲对象
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tbo, 0);

	// 颜色缓冲的独写对象改为GL_NONE，也就是不会读写颜色缓冲
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// 检查帧缓冲对象完整性
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//创建自定义帧缓冲MSAA
void CreateFrameBuffer_MSAA(GLuint& fbo, GLuint* tbo, GLuint& rbo, GLuint tbo_num)
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// 只有默认缓冲才能输出图像(因为和GLFW窗口绑定)，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 生成MSAA纹理附件 对应color缓冲
	glGenTextures(tbo_num, tbo);
	
	for (uint i = 0; i < tbo_num; i++)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbo[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLE_NUM, GL_RGB16F, windowWidth, windowHeight, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		// MSAA纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT0附件 附加到 帧缓冲对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, tbo[i], 0);
	}

	// 生成渲染缓冲对象 对应stencil，depth缓冲
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// stencil，depth和color缓冲应该都要对应MSAA，如果color是对应了MSAA 而stencil，depth没有对应MSAA，stencil depth和color是不匹配的，导致错误
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLE_NUM, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); 
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 渲染缓冲对象 作为一个GL_DEPTH_STENCIL_ATTACHMENT附件 附加到 帧缓冲上
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// 指定写入的颜色附件
	if (tbo_num == 2)
	{
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	}

	// 检查帧缓冲对象完整性
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//创建乒乓帧缓冲
void CreateFrameBuffer_pingpong()
{
	glGenFramebuffers(2, fbo_pingpong);
	glGenTextures(2, tbo_pingpong);

	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_pingpong[i]);

		glBindTexture(GL_TEXTURE_2D, tbo_pingpong[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_pingpong[i], 0);
	}

	// 检查帧缓冲对象完整性
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SetUniformBuffer()
{
	// view矩阵 world -> view
	mat4 view;
	myCam.setCamView();
	view = myCam.getCamView();
	
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(view)); //调用glBufferSubData填充数据之前要确保已经调用glBufferData分配了内存

	// 投影矩阵 view -> clip
	mat4 projection;
	float fov = myCam.getCamFov();

	projection = perspective(radians(fov), (float)windowWidth / (float)windowHeight, myCam.camNear, myCam.camFar); // 之前写成(float)(WINDOW_WIDTH / WINDOW_HEIGHT)了，精度丢失，导致结果是1
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(projection));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool InitOpenGL()
{
	// 初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLE_NUM); // 和窗口绑定的采样点，显然，窗口对应的是默认帧缓冲

	// 绘制窗口
	window = glfwCreateWindow(windowWidth, windowHeight, "koalahjf", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();
		return false;
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
		return false;
	}
	glViewport(0, 0, windowWidth, windowHeight);

	return true;
}

void DrawDepthMap()
{
	// 绘制depthmmap 
	// 
	// view
	mat4 view = lookAt(-dirLight_direction, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	// projection
	float near_plane = 1.0f, far_plane = 20.0f;
	mat4 projection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	mat4 dirLightSpace = projection * view;

	// Set Uniform To Shader
	scene.depthmapShader.Use();
	scene.depthmapShader.SetMat4("dirLightSpace", dirLightSpace);

	glViewport(0, 0, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthmap);

	if (bFrontFaceCulling)
	{
		glCullFace(GL_FRONT); //因为shadow acne只有在一个表面能同时被我们眼睛和光线看到的情况下才可能发生，因此直接将光线看到的表面引导到其他面去，就可以从本质上预防shadow acne。
		scene.DrawScene(true, false);
		glCullFace(GL_BACK);
	}
	else
	{
		scene.DrawScene(true, false);
	}

	scene.lightShader.Use();
	scene.lightShader.SetMat4("dirLightSpace", dirLightSpace);
	scene.lightShader.SetInt("depthMap", 30);
	glActiveTexture(GL_TEXTURE30);
	glBindTexture(GL_TEXTURE_2D, tbo_depthmap);
}

void DrawDepthCubemap(vec3 lightPos)
{
	// 绘制depthCubemap 
	// 
	// projection
	float aspect = (float)SHADOW_RESOLUTION_WIDTH / (float)SHADOW_RESOLUTION_HEIGHT;
	float near = 1.0f;
	float far = fFarPlanePt;
	mat4 projection = perspective(radians(90.0f), aspect, near, far);

	// view
	vector<mat4> transforms;
	transforms.push_back(projection * lookAt(lightPos, lightPos + vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(lightPos, lightPos + vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(lightPos, lightPos + vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f, 1.0f)));
	transforms.push_back(projection * lookAt(lightPos, lightPos + vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f,-1.0f)));
	transforms.push_back(projection * lookAt(lightPos, lightPos + vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(lightPos, lightPos + vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));
	
	// Set Uniform To Shader
	scene.depthCubemapShader.Use();

	for (int i = 0; i < 6; i++)
	{
		stringstream ss;
		ss << "ptLightSpace[" << i << "]";
		string target = ss.str();
		scene.depthCubemapShader.SetMat4(target, transforms[i]);
	}

	scene.depthCubemapShader.SetVec3("lightPos", lightPos);
	scene.depthCubemapShader.SetFloat("farPlane", far);

	glViewport(0, 0, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthCubemap);

	if (bFrontFaceCulling)
	{
		glCullFace(GL_FRONT); //因为shadow acne只有在一个表面能同时被我们眼睛和光线看到的情况下才可能发生，因此直接将光线看到的表面引导到其他面去，就可以从本质上预防shadow acne。
		scene.DrawScene(false, true);
		glCullFace(GL_BACK);
	}
	else
	{
		scene.DrawScene(false, true);
	}

	scene.lightShader.Use();
	scene.lightShader.SetVec3("PtLightPos", lightPos);
	scene.lightShader.SetInt("depthCubemap", 20);
	scene.lightShader.SetFloat("farPlane", far);
	scene.lightShader.SetBool("bDepthCubemapDebug", bDepthCubemapDebug);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo_depthCubemap);
}

void SetAllUniformValues()
{
	SetUniformBuffer();
	SetUniformToShader(scene.lightShader);
	SetUniformToShader(scene.screenShader);
	SetUniformToShader(scene.cubemapShader);
	SetUniformToShader(scene.reflectShader);
	SetUniformToShader(scene.refractShader);
	SetUniformToShader(scene.normalShader);
	SetUniformToShader(scene.lightInstShader);
}

void DrawScreen()
{
	// 关掉自定义缓冲的读写，就切换成了默认缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	// 清空各个缓冲区
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); //离屏渲染不需要glClear(GL_COLOR_BUFFER_BIT);

	// 主屏幕
	GLuint t_dummy = 0;
	const vector<Texture> screenTexture =
	{
		{tbo_middle[0], "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};
	const vector<Texture> screenTextureBright =
	{
		{tbo_middle[1], "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};


	scene.screen.SetTextures(screenTexture);
	scene.screen.AddTextures(screenTextureBright);

	bool horizontal = true;

	uint amount = 20;
	for (uint i = 0; i < amount; i++)
	{
		// 本次循环图片输出到 fbo_pingpong[horizontal]
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_pingpong[horizontal]);

		// 设置screenShader的horizontal,判断当前应该是横向计算还是纵向计算
		scene.screenShader.SetBool("horizontal", horizontal);

		// 第一次用原来的亮度图即可，之后都是上一次循环的pingpong渲染的输出结果了
		if (i != 0)
		{
			// tbo_middle[1]不变，只不过内容变成上一次循环的pingpong渲染的输出结果了
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_pingpong[!horizontal]);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_middle);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		scene.screen.DrawMesh(scene.screenShader, GL_TRIANGLES);

		horizontal = !horizontal;
	}

	// 关掉自定义缓冲的读写，就切换成了默认缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene.screen.DrawMesh(scene.screenShader, GL_TRIANGLES);


	// 后视镜
	const vector<Texture> mirrorTexture =
	{
		{tbo_mirror[0], "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};
	scene.mirror.SetTextures(mirrorTexture);
	scene.mirror.DrawMesh(scene.screenShader, GL_TRIANGLES);


	if (bShadow && bDisDepthmap)
	{
		glViewport(0, 0, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT);
		const vector<Texture> depthmapTexture =
		{
			{tbo_depthmap, "texture_diffuse"},
			{t_dummy, "texture_specular"}

		};
		scene.screen.SetTextures(depthmapTexture);
		scene.screen.DrawMesh(scene.depthmapDisplayShader, GL_TRIANGLES);
		glViewport(0, 0, windowWidth, windowHeight);
	}
}


//创建 G-buffer
void CreateFrameBuffer_G()
{
	// 首先创建一个帧缓冲对象 （由color stencil depth组成。默认缓冲区也有。只不过这次自己创建缓冲区，可以实现一些有意思的功能）
	// 只有默认缓冲才能输出图像(因为和GLFW窗口绑定)，用自建的缓冲不会输出任何图像，因此可以用来离屏渲染
	glGenFramebuffers(1, &fbo_G);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_G);

	/**********************position缓冲，储存像素的位置信息*********************/
	// 生成纹理附件 对应color缓冲
	glGenTextures(1, &tbo_G_position);

	// 设置纹理参数
	glBindTexture(GL_TEXTURE_2D, tbo_G_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT附件 附加到 帧缓冲对象
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_G_position, 0);

	/**********************normal缓冲，储存像素的法线信息*********************/

	// 生成纹理附件 对应color缓冲
	glGenTextures(1, &tbo_G_normal);

	// 设置纹理参数
	glBindTexture(GL_TEXTURE_2D, tbo_G_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT附件 附加到 帧缓冲对象
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tbo_G_normal, 0);

	/**********************albedo and specular缓冲，储存像素的反照率和高光信息*********************/

	// 生成纹理附件 对应color缓冲
	glGenTextures(1, &tbo_G_abdspec);

	// 设置纹理参数
	glBindTexture(GL_TEXTURE_2D, tbo_G_abdspec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 纹理缓冲对象  作为一个GL_COLOR_ATTACHMENT附件 附加到 帧缓冲对象
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tbo_G_abdspec, 0);

	// 设置渲染到本buffer的三个color附件
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(3, attachments);

	// 生成渲染缓冲对象 对应stencil，depth缓冲
	glGenRenderbuffers(1, &rbo_G);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_G);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 渲染缓冲对象 作为一个GL_DEPTH_STENCIL_ATTACHMENT附件 附加到 帧缓冲上
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_G);

	// 检查帧缓冲对象完整性
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
