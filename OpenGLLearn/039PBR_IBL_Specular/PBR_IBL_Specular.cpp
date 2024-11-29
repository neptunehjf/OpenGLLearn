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
void CreateFrameBuffer_G_SSAO();
void CreateFrameBuffer_SSAO_Output();
void CreateFrameBuffer_SSAO_Blur();
void CreateFrameBuffer_EnvCubemap();
void CreateFrameBuffer_IrdCubemap();
void CreateFrameBuffer_PrefilterCubemap();
void CreateFrameBuffer_BRDF();
void SetUniformBuffer();
void DrawDepthMap();
void DrawDepthCubemap(vec3 lightPos);
void SetAllUniformValues();
void DrawScreen();
void DrawSceneDeffered();
void DrawSceneSSAO();
void DrawSceneSSAOBlur();
void SetHeavyLightsUniform(Shader& shader);
void SetPBRUniform();
void SetPBRWithTextureUniform();
void DrawEnvCubemap();
void SkyBoxTest(Mesh& skybox);
void DrawIrradianceCubemap();
void DrawPrefilterCubemap();
void DrawBRDF();

Scene scene;
Camera myCam(vec3(0.2f, 0.16f, 0.35f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
GLFWwindow* window = NULL;

// ԭ��������
GLuint fbo_origin = 0; // �Զ���֡�������
GLuint tbo_origin[2] = {}; // ��������󣨸�����
GLuint rbo_origin = 0; // ��Ⱦ������󣨸�����

// ���Ӿ�����
GLuint fbo_mirror = 0; // �Զ���֡�������
GLuint tbo_mirror[1] = {}; // ��������󣨸�����
GLuint rbo_mirror = 0; // ��Ⱦ������󣨸�����

// �м仺��
GLuint fbo_middle = 0; // �Զ���֡�������
GLuint tbo_middle[2] = {}; // ��������󣨸�����
GLuint rbo_middle = 0; // ��Ⱦ������󣨸�����

// depthmap����
GLuint fbo_depthmap = 0; // �Զ���֡�������
GLuint tbo_depthmap = 0; // ��������󣨸�����

// depthCubemap����
GLuint fbo_depthCubemap = 0; // �Զ���֡�������
GLuint tbo_depthCubemap = 0; // ��������󣨸�����

// Uniform����
GLuint ubo = 0;

// pingpong����
GLuint fbo_pingpong[2] = {}; // �Զ���֡�������
GLuint tbo_pingpong[2] = {}; // ��������󣨸�����

// G����
GLuint fbo_G = 0;             // �Զ���֡�������
GLuint tbo_G_position = 0;    // ��������󣨸����� �洢λ����Ϣ
GLuint tbo_G_normal = 0;      // ��������󣨸����� �洢������Ϣ
GLuint tbo_G_abdspec = 0;     // ��������󣨸����� �洢�����ʺ͸߹���Ϣ
GLuint rbo_G = 0;             // ��Ⱦ������󣨸�����

// deffered shading ����
GLuint fbo_deffered = 0; // �Զ���֡�������
GLuint tbo_deffered[2] = {}; // ��������󣨸�����
GLuint rbo_deffered = 0; // ��Ⱦ������󣨸�����

// G���� SSAO input (��G����������SSAO���������)
GLuint fbo_SSAO = 0;                // �Զ���֡�������
GLuint tbo_SSAO_posdepth = 0;       // ��������󣨸����� �洢λ����Ϣ
GLuint tbo_SSAO_normal = 0;         // ��������󣨸����� �洢������Ϣ
GLuint tbo_SSAO_albedo = 0;         // ��������󣨸����� �洢��������Ϣ
GLuint rbo_SSAO = 0;                // ��Ⱦ������󣨸�����

// SSAO���� SSAO output (��SSAO��������)
GLuint fbo_SSAO_out = 0;                // �Զ���֡�������
GLuint tbo_SSAO_out = 0;                // ��������󣨸����� �洢occlusion��Ϣ

// SSAO blur���� ����ƽ������
GLuint fbo_SSAO_blur = 0;                // �Զ���֡�������
GLuint tbo_SSAO_blur = 0;                // ��������󣨸����� �洢ƽ���������occlusion��Ϣ

// PBR IBL ����Cubemap
GLuint fbo_EnvCubemap = 0;               // �Զ���֡�������
GLuint tbo_EnvCubemap = 0;               // ��������󣨸�����

// PBR IBL �����Cubemap
GLuint fbo_irdCubemap = 0;               // �Զ���֡�������
GLuint tbo_irdCubemap = 0;               // ��������󣨸�����

// PBR IBL Ԥ�˲�(pre filter)Cubemap
GLuint fbo_pfCubemap = 0;                // �Զ���֡�������
GLuint tbo_pfCubemap = 0;                // ��������󣨸�����

// PBR IBL BRDF ����
GLuint fbo_BRDF = 0;                     // �Զ���֡�������
GLuint tbo_BRDF = 0;                     // ��������󣨸�����

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

	// ԭ��������
	CreateFrameBuffer_MSAA(fbo_origin, tbo_origin, rbo_origin, 2);
	// ���Ӿ�����
	CreateFrameBuffer(fbo_mirror, tbo_mirror, rbo_mirror, 1);
	// �м仺��
	CreateFrameBuffer(fbo_middle, tbo_middle, rbo_middle, 2);
	// depthmap����
	CreateFrameBuffer_Depthmap(fbo_depthmap, tbo_depthmap);
	// depthCubemap����
	CreateFrameBuffer_DepthCubemap(fbo_depthCubemap, tbo_depthCubemap);
	// G����
	CreateFrameBuffer_G();
	// deferred shading ����
	CreateFrameBuffer(fbo_deffered, tbo_deffered, rbo_deffered, 2);
	// G���� SSAO
	CreateFrameBuffer_G_SSAO();
	// SSAO��� ����
	CreateFrameBuffer_SSAO_Output();
	// SSAOģ������
	CreateFrameBuffer_SSAO_Blur();
	// ������������ͼ����
	CreateFrameBuffer_EnvCubemap();
	// �������������ͼ����
	CreateFrameBuffer_IrdCubemap();
	// Ԥ�˲���������ͼ����
	CreateFrameBuffer_PrefilterCubemap();
	// BRDFԤ������ͼ����
	CreateFrameBuffer_BRDF();

	// Uniform����
	// 
	// ����UniformBlockIndex�󶨸�shader��uniformblock��binding point����opengl 420���ϰ汾����ֱ����layout(std140, binding = XXX)��shaderֱ��ָ�� 
	GLuint ubi_Lighting = glGetUniformBlockIndex(scene.lightShader.ID, "Matrix");
	GLuint ubi_Cubemap = glGetUniformBlockIndex(scene.cubemapShader.ID, "Matrix");
	GLuint ubi_Refract  = glGetUniformBlockIndex(scene.refractShader.ID, "Matrix");
	GLuint ubi_Reflect  = glGetUniformBlockIndex(scene.reflectShader.ID, "Matrix");

	glUniformBlockBinding(scene.lightShader.ID, ubi_Lighting, 0);
	glUniformBlockBinding(scene.cubemapShader.ID, ubi_Cubemap, 0);
	glUniformBlockBinding(scene.refractShader.ID, ubi_Refract, 0);
	glUniformBlockBinding(scene.reflectShader.ID, ubi_Reflect, 0);

	// ����Uniform�����������󶨵���Ӧ��binding point
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_STATIC_DRAW); // ֻ��4->16�������Ҫ�����ڴ���롣NULL��ʾֻ�����ڴ棬��д�����ݡ�
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	bool bLastGamma = false; 
	bool bLastNM = false;
	bool bLastHDR = false;
	
	// ���ƻ�����������ͼ�����浽�Զ��建����
	scene.CreateScene(&myCam);

	DrawEnvCubemap();

	const vector<Texture> EnvTexture =
	{
		{tbo_EnvCubemap, "texture_cubemap"}
	};
	scene.cube_irradiance = Mesh(g_cubeVertices, g_cubeIndices, EnvTexture);
	DrawIrradianceCubemap();

	scene.cube_prefilter = Mesh(g_cubeVertices, g_cubeIndices, EnvTexture);
	DrawPrefilterCubemap();

	const vector<Texture> irradianceTexture =
	{
		{tbo_irdCubemap, "texture_cubemap"}
	};
	const vector<Texture> prefilterTexture =
	{
		{tbo_pfCubemap, "texture_cubemap"}
	};
	scene.sphere = scene.CreateSphereMesh(irradianceTexture);

	DrawBRDF();
	const vector<Texture> BRDFTexture =
	{
		{tbo_BRDF, "texture_diffuse"}
	};

	//test skybox
	scene.skybox = Mesh(g_skyboxVertices, g_skyboxIndices, prefilterTexture);

	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))
	{
		if (bGammaCorrection)
		{
			glEnable(GL_FRAMEBUFFER_SRGB); //��ɫд��֡����֮ǰ�ᱻgammaУ��
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

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		if (bMSAA)
			glEnable(GL_MULTISAMPLE); // �ܶ�opengl��������Ҫ��ʽ�ص��ã����ǻ��ǵ���һ�±���һ��
		else 
			glDisable(GL_MULTISAMPLE);

		curTime = glfwGetTime();
		deltaTime = curTime - preTime;
		preTime = curTime;
		/********************** �����Զ���֡�������������Ⱦ �󶨵��Զ���֡���壬Ĭ��֡���岻�������� **********************/

		if (bShadow)
		{
			DrawDepthMap();
			DrawDepthCubemap(lampWithShadowPos);
		}

		// ԭ����
		glViewport(0, 0, windowWidth, windowHeight);

		/*************************Output G-Buffer Info****************************/
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_origin);
		SetAllUniformValues();

		// ����պв��Ի�����ͼ��û����ȷ���浽cubemap��
		SkyBoxTest(scene.skybox);

		scene.DrawScene_PBR();

		//if (!bDeferred)
		//{
		//	SetHeavyLightsUniform(scene.ForwardShader);
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_origin);
		//	scene.DrawScene_DeferredTest();
		//	//scene.DrawScene();
		//}
		//else
		//{
		//	SetHeavyLightsUniform(scene.DeferredShader);
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_G);
		//	scene.DrawScene_DeferredTest();
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_deffered);
		//	DrawSceneDeffered();
		//}

		/*************************Output G-Buffer SSAO Info****************************/
		//if (bSSAO)
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_SSAO);
		//	scene.DrawScene_SSAOTest();
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_SSAO_out);
		//	DrawSceneSSAO();
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_SSAO_blur);
		//	DrawSceneSSAOBlur();
		//	glBindFramebuffer(GL_FRAMEBUFFER, fbo_deffered);
		//	DrawSceneDeffered();
		//}
			
		// ���м�fbo�ķ�ʽʵ�֣�ʵ�����м�FBO����һ��ֻ��1�����������ͨ֡���塣��Blit������MSAA FBO���ƽ�ȥ��Ȼ��Ϳ������м�FBO��TBO�����ڴ����ˡ�
		// ȱ���ǣ�����ڴ˻����Ͻ��к���ȥ������ɫ������1�������������Ϊ����������ģ����ܻ����µ��¾��
		if (!bDeferred)
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_origin);
		else
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_deffered);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_middle);

		// MSAA��������ʵҲ����ֱ�Ӵ�����ɫ�����в�������ΪMSAA�����ʽ����ͨ����һ�������Բ�������ͨ�������ֱ���á�
		// ������sampler2DMS�ķ�ʽ���룬���Ի�ȡ��ÿ�������㣬��Ҫ�����Զ��忹����㷨
		// ֻ�ǵ�����Ⱦ�����Ļ�����glBlitFramebuffer�͹���
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer(0);
		glDrawBuffer(0);

		// ���Ӿ�����
		if (bBackMirror)
		{
			myCam.yawValue += 180.0;
			SetAllUniformValues();
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_mirror);
			scene.DrawScene();
			myCam.yawValue -= 180.0;
			SetAllUniformValues();
			myCam.yawValue = myCam.yawValue - ((int)myCam.yawValue / 360) * 360;
		}

		/********************** Ĭ��֡�������ǰ�����ʱд�� **********************/
		DrawScreen();

		glDisable(GL_FRAMEBUFFER_SRGB); //imgui���治��ҪgammaУ��
		// imgui��Ĭ�ϻ����л��ƣ���Ϊ�Ҳ���imguiҲ�к��ڴ���Ч��
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ���������� ��ѯ�¼�
		glfwSwapBuffers(window);
	}

	// ��Դ����
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

	// bugfix ���ڴ�С�仯��������Ĵ�СҲҪ��Ӧ�仯
	// ɾ���Ѵ��ڵĻ���
	glDeleteFramebuffers(1, &fbo_origin);
	glDeleteFramebuffers(1, &fbo_mirror);
	glDeleteFramebuffers(1, &fbo_middle);
	glDeleteFramebuffers(1, &fbo_G);
	glDeleteFramebuffers(1, &fbo_deffered);
	glDeleteFramebuffers(1, &fbo_SSAO);
	glDeleteFramebuffers(1, &fbo_SSAO_out);

	// ԭ��������
	CreateFrameBuffer_MSAA(fbo_origin, tbo_origin, rbo_origin, 2);
	// ���Ӿ�����
	CreateFrameBuffer(fbo_mirror, tbo_mirror, rbo_mirror, 1);
	// �м仺��
	CreateFrameBuffer(fbo_middle, tbo_middle, rbo_middle, 2);
	// G����
	CreateFrameBuffer_G();
	// deferred shading ����
	CreateFrameBuffer(fbo_deffered, tbo_deffered, rbo_deffered, 2);
	// G���� SSAO
	CreateFrameBuffer_G_SSAO();
	// SSAO��� ����
	CreateFrameBuffer_SSAO_Output();
	// SSAOģ������
	CreateFrameBuffer_SSAO_Blur();

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
	// ����Ҽ������Ͳ�������Ϊ���Ҫ������Imgui
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		return;
	}

	/* ��ͷ���� */
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
		myCam.camFar = imgui_camFar; // �����������������imgui���������뱣֤���߳�ʼֵһ�£���Ϊimgui�����״̬�ǲ���ֵ�ġ�

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Lighting"))
	{
		// ����ģ��
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
		ImGui::Checkbox("Show Back Mirror", &bBackMirror);
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

	if (ImGui::TreeNodeEx("HDR"))
	{
		ImGui::Checkbox("Enable HDR", &bHDR);
		const char* aAlgro[] = { "reinhard tone mapping", "exposure tone mapping" };
		ImGui::Combo("Tone Mapping Algorithm", &iHDRAlgro, aAlgro, IM_ARRAYSIZE(aAlgro));

		if (iHDRAlgro == 1)
			ImGui::SliderFloat("Exposure", &fExposure, 0.0f, 5.0f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Bloom"))
	{
		ImGui::Checkbox("Enable Bloom", &bBloom);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Deferred Shading"))
	{
		ImGui::Checkbox("Enable Deferred Shading", &bDeferred);
		if (bDeferred)
		{
			ImGui::Checkbox("Combine With Forward Shading", &bCombined);
		}
		ImGui::Checkbox("Light Volume", &bLightVolume);
		ImGui::SliderInt("GPU Pressure", &iGPUPressure, 1, 10);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Screen Based Ambient Occlusion"))
	{
		ImGui::Checkbox("Enable SSAO", &bSSAO);
		ImGui::SliderInt("SSAO Samples Number", &iSSAOSampleNum, 1, 256);
		ImGui::Checkbox("Enable Nosie", &bSSAONoise);
		ImGui::SliderInt("Noise Strength", &iSSAONoise, 2, 20);
		ImGui::SliderFloat("Radius", &fRadius, 0.1, 20);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("PBR", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("metallic", &metallic, 0.0, 1.0);
		ImGui::SliderFloat("roughness", &roughness, 0.0, 1.0);
		ImGui::SliderFloat("ao", &ao, 0.0, 1.0);
		ImGui::Checkbox("Enable Image Based Lighting", &bIBL);
		ImGui::SliderInt("Skybox Mipmap Level", &iMipLevel, 0, 4);

		const char* aMode[] = { "FresnelSchlick", "FresnelSchlickRoughness" };
		ImGui::Combo("Frensel Mode", &iFrenselMode, aMode, IM_ARRAYSIZE(aMode));

		ImGui::TreePop();
	}
}

void SetUniformToShader(Shader& shader)
{
	//����lightShader���� �����漰����shader������л�������ÿ��loop�ﶼҪ�ڶ�Ӧ��λ�õ��ã�����ֻ�ڿ�ʼ����һ��
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
	shader.SetFloat("near", myCam.camNear);
	shader.SetFloat("far", myCam.camFar);
	shader.SetBool("bSSAO", bSSAO);
	shader.SetInt("samples_num", iSSAOSampleNum);
	shader.SetInt("iSSAONoise", iSSAONoise);
	shader.SetInt("iMipLevel", iMipLevel);

	for (int i = 0; i < scene.ssaoKernel.size(); i++)
	{
		stringstream ss;
		ss << "samples[" << i << "]";
		string prefix = ss.str();

		shader.SetVec3(prefix, scene.ssaoKernel[i]);
	}
	shader.SetFloat("fRadius", fRadius);
	
	// ShaderLightingInstance 
	// ��Ϊmodel����任�ǻ��ڵ�λ������еģ���Ҫ���Ѿ��任���model����Ļ����ϣ��ٽ���model����任�е�����
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
		shader.SetFloat(prefix + "constant", constant);
		shader.SetFloat(prefix + "linear", linear);
		shader.SetFloat(prefix + "quadratic", quadratic);
	}

	shader.SetVec3("pointLight[4].lightPos", lampWithShadowPos); 
	shader.SetVec3("pointLight[4].ambient", pointLight_ambient);
	shader.SetVec3("pointLight[4].diffuse", pointLight_diffuse);
	shader.SetVec3("pointLight[4].specular", pointLight_specular);
	shader.SetFloat("pointLight[4].constant", constant);
	shader.SetFloat("pointLight[4].linear", linear);
	shader.SetFloat("pointLight[4].quadratic", quadratic);
}

//�����Զ���֡����
void CreateFrameBuffer(GLuint& fbo, GLuint* tbo, GLuint& rbo, GLuint tbo_num)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// ���������� ��Ӧcolor����
	glGenTextures(tbo_num, tbo);

	for (uint i = 0; i < tbo_num; i++)
	{
		glBindTexture(GL_TEXTURE_2D, tbo[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT0���� ���ӵ� ֡�������
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tbo[i], 0);	
	}

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// ��Ⱦ������� ��Ϊһ��GL_DEPTH_STENCIL_ATTACHMENT���� ���ӵ� ֡������
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	
	// ָ��д�����ɫ����
	if (tbo_num == 2)
	{
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	}

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//�����Զ���֡���� depthmap
void CreateFrameBuffer_Depthmap(GLuint& fbo, GLuint& tbo)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// ���������� ��ӦDepth����
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	// ���ͼ����������Ӧ���� GL_FLOAT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_DEPTH_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tbo, 0);

	// ��ɫ����Ķ�д�����ΪGL_NONE��Ҳ���ǲ����д��ɫ����
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//�����Զ���֡���� depth cubemap
void CreateFrameBuffer_DepthCubemap(GLuint& fbo, GLuint& tbo)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// ���������� ��Ӧdepth����
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo);
	for (uint i = 0; i < 6; i++)
	{
		// ���ͼ����������Ӧ���� GL_FLOAT
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// ���������  ��Ϊһ��GL_DEPTH_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tbo, 0);

	// ��ɫ����Ķ�д�����ΪGL_NONE��Ҳ���ǲ����д��ɫ����
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//�����Զ���֡����MSAA
void CreateFrameBuffer_MSAA(GLuint& fbo, GLuint* tbo, GLuint& rbo, GLuint tbo_num)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// ����MSAA������ ��Ӧcolor����
	glGenTextures(tbo_num, tbo);
	
	for (uint i = 0; i < tbo_num; i++)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbo[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLE_NUM, GL_RGB16F, windowWidth, windowHeight, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		// MSAA���������  ��Ϊһ��GL_COLOR_ATTACHMENT0���� ���ӵ� ֡�������
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, tbo[i], 0);
	}

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// stencil��depth��color����Ӧ�ö�Ҫ��ӦMSAA�����color�Ƕ�Ӧ��MSAA ��stencil��depthû�ж�ӦMSAA��stencil depth��color�ǲ�ƥ��ģ����´���
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLE_NUM, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); 
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// ��Ⱦ������� ��Ϊһ��GL_DEPTH_STENCIL_ATTACHMENT���� ���ӵ� ֡������
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// ָ��д�����ɫ����
	if (tbo_num == 2)
	{
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	}

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//����ƹ��֡����
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

	// ���֡�������������
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
	// view���� world -> view
	mat4 view;
	myCam.setCamView();
	view = myCam.getCamView();
	
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(view)); //����glBufferSubData�������֮ǰҪȷ���Ѿ�����glBufferData�������ڴ�

	// ͶӰ���� view -> clip
	mat4 projection;
	float fov = myCam.getCamFov();

	projection = perspective(radians(fov), (float)windowWidth / (float)windowHeight, myCam.camNear, myCam.camFar); // ֮ǰд��(float)(WINDOW_WIDTH / WINDOW_HEIGHT)�ˣ����ȶ�ʧ�����½����1
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(projection));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool InitOpenGL()
{
	// ��ʼ��
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLE_NUM); // �ʹ��ڰ󶨵Ĳ����㣬��Ȼ�����ڶ�Ӧ����Ĭ��֡����

	// ���ƴ���
	window = glfwCreateWindow(windowWidth, windowHeight, "koalahjf", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();
		return false;
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
		return false;
	}
	glViewport(0, 0, windowWidth, windowHeight);

	return true;
}

void DrawDepthMap()
{
	// ����depthmmap 
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

	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthmap);

	if (bFrontFaceCulling)
	{
		glCullFace(GL_FRONT); //��Ϊshadow acneֻ����һ��������ͬʱ�������۾��͹��߿���������²ſ��ܷ��������ֱ�ӽ����߿����ı���������������ȥ���Ϳ��Դӱ�����Ԥ��shadow acne��
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
	// ����depthCubemap 
	// 
	// projection
	float aspect = 1.0f;
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

	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthCubemap);

	if (bFrontFaceCulling)
	{
		glCullFace(GL_FRONT); //��Ϊshadow acneֻ����һ��������ͬʱ�������۾��͹��߿���������²ſ��ܷ��������ֱ�ӽ����߿����ı���������������ȥ���Ϳ��Դӱ�����Ԥ��shadow acne��
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
	SetUniformToShader(scene.GBufferShader);
	SetUniformToShader(scene.DeferredShader);
	SetUniformToShader(scene.GBufferSSAOShader);
	SetUniformToShader(scene.SSAOShader);
	SetUniformToShader(scene.SSAOBlurShader);
	SetPBRUniform();
	SetPBRWithTextureUniform();
}

void DrawScreen()
{
	// �ص��Զ��建��Ķ�д�����л�����Ĭ�ϻ���
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	// ��ո���������
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); //������Ⱦ����ҪglClear(GL_COLOR_BUFFER_BIT);

	// ����Ļ
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
		// ����ѭ��ͼƬ����� fbo_pingpong[horizontal]
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_pingpong[horizontal]);

		// ����screenShader��horizontal,�жϵ�ǰӦ���Ǻ�����㻹���������
		scene.screenShader.SetBool("horizontal", horizontal);

		// ��һ����ԭ��������ͼ���ɣ�֮������һ��ѭ����pingpong��Ⱦ����������
		if (i != 0)
		{
			// tbo_middle[1]���䣬ֻ�������ݱ����һ��ѭ����pingpong��Ⱦ����������
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_pingpong[!horizontal]);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_middle);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		scene.screen.DrawMesh(scene.screenShader, GL_TRIANGLES);

		horizontal = !horizontal;
	}

	// �ص��Զ��建��Ķ�д�����л�����Ĭ�ϻ���
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene.screen.DrawMesh(scene.screenShader, GL_TRIANGLES);


	// ���Ӿ�
	if (bBackMirror)
	{
		const vector<Texture> mirrorTexture =
		{
			{tbo_mirror[0], "texture_diffuse"},
			{t_dummy, "texture_specular"}
		};
		scene.mirror.SetTextures(mirrorTexture);
		scene.mirror.DrawMesh(scene.screenShader, GL_TRIANGLES);
	}

	if (bShadow && bDisDepthmap)
	{
		glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
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

//���� G-buffer
void CreateFrameBuffer_G()
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo_G);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_G);

	/**********************position���壬�������ص�λ����Ϣ*********************/
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_G_position);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_G_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ��GL_CLAMP_TO_EDGE��ֹ��������Ļ�ռ�֮������ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_G_position, 0);

	/**********************normal���壬�������صķ�����Ϣ*********************/

	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_G_normal);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_G_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ��GL_CLAMP_TO_EDGE��ֹ��������Ļ�ռ�֮������ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tbo_G_normal, 0);

	/**********************albedo and specular���壬�������صķ����ʺ͸߹���Ϣ*********************/

	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_G_abdspec);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_G_abdspec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ��GL_CLAMP_TO_EDGE��ֹ��������Ļ�ռ�֮������ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tbo_G_abdspec, 0);

	// ������Ⱦ����buffer������color����
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo_G);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_G);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// ��Ⱦ������� ��Ϊһ��GL_DEPTH_STENCIL_ATTACHMENT���� ���ӵ� ֡������
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_G);

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//���� G-buffer SSAO
void CreateFrameBuffer_G_SSAO()
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo_SSAO);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_SSAO);

	/**********************posdepth���壬�������ص�λ����Ϣ�������Ϣ*********************/
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_SSAO_posdepth);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_SSAO_posdepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ��GL_CLAMP_TO_EDGE��ֹ��������Ļ�ռ�֮������ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_SSAO_posdepth, 0);

	/**********************normal���壬�������صķ�����Ϣ*********************/

	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_SSAO_normal);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_SSAO_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ��GL_CLAMP_TO_EDGE��ֹ��������Ļ�ռ�֮������ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tbo_SSAO_normal, 0);

	/**********************albedo and specular���壬�������صķ����ʺ͸߹���Ϣ*********************/

	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_SSAO_albedo);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_SSAO_albedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ��GL_CLAMP_TO_EDGE��ֹ��������Ļ�ռ�֮������ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tbo_SSAO_albedo, 0);

	// ������Ⱦ����buffer������color����
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo_SSAO);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_SSAO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// ��Ⱦ������� ��Ϊһ��GL_DEPTH_STENCIL_ATTACHMENT���� ���ӵ� ֡������
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_SSAO);

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawSceneDeffered()
{
	glDisable(GL_DEPTH_TEST);

	// ��ո���������
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); //������Ⱦ����ҪglClear(GL_COLOR_BUFFER_BIT);

	// ����Ļ
	const vector<Texture> gBufferTexture =
	{
		{tbo_G_position, "texture_diffuse"},
		{tbo_G_normal, "texture_diffuse"},
		{tbo_G_abdspec, "texture_diffuse"},
		{tbo_SSAO_blur, "texture_diffuse"}
	};
	scene.defferedScreen.SetTextures(gBufferTexture);
	scene.defferedScreen.DrawMesh(scene.DeferredShader, GL_TRIANGLES);

	if (bDeferred && bCombined)
	{
		glEnable(GL_DEPTH_TEST);

		// ȡ��G-buffer�е������Ϣ
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_G);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_deffered);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		// ��Ҫʵ�ְ�mesh��Ⱦ��ֻ��deferred shading���в�ͨ�ģ���Ϊdeferred shadingֻ�ܰ���Ļ������Ⱦ���ֲ������ĸ�mesh
		// ��ȻҲ����Ϊÿ��meshָ��һ����ɫ��ͼ������̫�鷳�Ҵ�ɫ��ͼ����ǻ��ٽ���һ�ι��ռ��㣬����Ҳ���Ǵ�ɫ
		// ��ʱ����԰�deferred shading�� forward shading���ʹ��
		// ���磬ÿ����Դ��drawһ����ɫ�������壨�����Ǽ�����գ������´��������deferred shading֮�����forward shading
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_deffered);

		for (uint i = 0; i < HEAVY_LIGHTS_NUM; i++)
		{
			scene.DeferredLampShader.SetVec3("lightColor", scene.lightColors[i]);
			scene.cube.SetScale(vec3(0.1f));
			scene.cube.SetTranslate(scene.lightPositions[i]);

			scene.cube.DrawMesh(scene.DeferredLampShader, GL_TRIANGLES);
		}
	}
}

void DrawSceneSSAO()
{
	glDisable(GL_DEPTH_TEST);

	// ��ո���������
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// ����Ļ
	const vector<Texture> gBufferTexture =
	{
		{tbo_SSAO_posdepth, "texture_diffuse"},
		{tbo_SSAO_normal, "texture_diffuse"},
		{scene.noiseTexture, "texture_diffuse"}
	};
	scene.SSAOScreen.SetTextures(gBufferTexture);
	scene.SSAOScreen.DrawMesh(scene.SSAOShader, GL_TRIANGLES);
}

void DrawSceneSSAOBlur()
{
	glDisable(GL_DEPTH_TEST);

	// ��ո���������
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// ����Ļ
	const vector<Texture> SSAOTexture =
	{
		{tbo_SSAO_out, "texture_diffuse"}
	};
	scene.SSAOBlurScreen.SetTextures(SSAOTexture);
	scene.SSAOBlurScreen.DrawMesh(scene.SSAOBlurShader, GL_TRIANGLES);
}

void SetHeavyLightsUniform(Shader &shader)
{
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

	for (int i = 0; i < HEAVY_LIGHTS_NUM; i++)
	{
		stringstream ss;
		ss << "pointLight[" << i << "].";
		string prefix = ss.str();

		shader.SetVec3(prefix + "lightPos", scene.lightPositions[i]);
		shader.SetVec3(prefix + "ambient", pointLight_ambient);
		shader.SetVec3(prefix + "diffuse", scene.lightColors[i]);
		shader.SetVec3(prefix + "specular", pointLight_specular);
		shader.SetFloat(prefix + "constant", constant);
		shader.SetFloat(prefix + "linear", linear);
		shader.SetFloat(prefix + "quadratic", quadratic);
		shader.SetFloat(prefix + "radius", scene.lightRadius[i]);
	}

	shader.SetInt("material.shininess", material_shininess);

	shader.SetBool("bLightVolume", bLightVolume);

	shader.SetInt("iGPUPressure", iGPUPressure);

}

void SetPBRUniform()
{
	scene.PBRShader.Use();

	vec3 lightPositions[] = {
		vec3(-10.0f,  10.0f, 10.0f),
		vec3(10.0f,  10.0f, 10.0f),
		vec3(-10.0f, -10.0f, 10.0f),
		vec3(10.0f, -10.0f, 10.0f),
	};
	vec3 lightColors[] = {
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f)
	};

	scene.PBRShader.SetFloat("metallic", metallic);
	scene.PBRShader.SetFloat("roughness", roughness);
	scene.PBRShader.SetFloat("ao", ao);
	
	scene.PBRShader.SetVec3("albedo", vec3(0.5f, 0.0f, 0.0f));
	scene.PBRShader.SetInt("iFrenselMode", iFrenselMode);

	for (int i = 0; i < 4; i++)
	{
		stringstream ss1, ss2;
		ss1 << "lightPositions[" << i << "]";
		ss2 << "lightColors[" << i << "]";
		string lp = ss1.str();
		string lc = ss2.str();

		scene.PBRShader.SetVec3(lp, lightPositions[i]);
		scene.PBRShader.SetVec3(lc, lightColors[i]);
	}

	scene.PBRShader.SetVec3("camPos", myCam.camPos);

	scene.PBRShader.SetBool("bIBL", bIBL);
}

void SetPBRWithTextureUniform()
{
	scene.PBRWithTextureShader.Use();

	vec3 lightPositions[] = {
		vec3(-10.0f,  10.0f, 10.0f),
		vec3(10.0f,  10.0f, 10.0f),
		vec3(-10.0f, -10.0f, 10.0f),
		vec3(10.0f, -10.0f, 10.0f),
	};
	vec3 lightColors[] = {
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f)
	};

	for (int i = 0; i < 4; i++)
	{
		stringstream ss1, ss2;
		ss1 << "lightPositions[" << i << "]";
		ss2 << "lightColors[" << i << "]";
		string lp = ss1.str();
		string lc = ss2.str();

		scene.PBRWithTextureShader.SetVec3(lp, lightPositions[i]);
		scene.PBRWithTextureShader.SetVec3(lc, lightColors[i]);
	}

	scene.PBRWithTextureShader.SetVec3("camPos", myCam.camPos);
}

//����SSAO��� ������
void CreateFrameBuffer_SSAO_Output()
{
	glGenFramebuffers(1, &fbo_SSAO_out);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_SSAO_out);

	/**********************color���壬�������ص�Occlusion��Ϣ*********************/
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_SSAO_out);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_SSAO_out);
	// Occlusion�õ�ͨ��float����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL); // Q: ����������������GL_REDҲ�аɣ�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_SSAO_out, 0);

	// ����Ҫrender buffer object

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//����SSAOģ�� ������
void CreateFrameBuffer_SSAO_Blur()
{
	glGenFramebuffers(1, &fbo_SSAO_blur);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_SSAO_blur);

	/**********************color���壬����ģ�����Occlusion��Ϣ*********************/
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_SSAO_blur);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_SSAO_blur);
	// Occlusion�õ�ͨ��float����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL); // Q: ����������������GL_REDҲ�аɣ�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_SSAO_blur, 0);

	// ����Ҫrender buffer object

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//�����Զ���֡���� IBL ����Cubemap
void CreateFrameBuffer_EnvCubemap()
{
	glGenFramebuffers(1, &fbo_EnvCubemap);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_EnvCubemap);

	// ���������� ��Ӧdepth����
	glGenTextures(1, &tbo_EnvCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo_EnvCubemap);
	for (uint i = 0; i < 6; i++)
	{
		// HDR����������Ӧ���� GL_FLOAT
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, ENVIRONMENT_RESOLUTION, ENVIRONMENT_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// color�����ȵ���Ⱦ��ʱ���ٹ���

	// depth��������Ҫ��

	// ���֡�������������
	// ��ΪҪ�ȵ���Ⱦ��ʱ����������������Դ˴����ж���������

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawEnvCubemap()
{
	// ���ƻ���Cubemap 

	// projection
	float aspect = 1.0f;
	float near = 0.1f; // �����׶�Ľ�ƽ�����ù��󣬻ᵼ�²����ƽ�����Ƭ��
	float far = 10.0f;
	mat4 projection = perspective(radians(90.0f), aspect, near, far);

	// view
	vector<mat4> transforms;
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));

	// Set Uniform To Shader
	scene.GetEquireColorShader.Use();

	glViewport(0, 0, ENVIRONMENT_RESOLUTION, ENVIRONMENT_RESOLUTION);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_EnvCubemap);

	for (uint i = 0; i < 6; i++)
	{
		scene.GetEquireColorShader.SetMat4("transforms", transforms[i]);

		// ������ÿdrawһ���棬����һ�����color attachment
		// Ҳ����һ��draw����shader��ֱ���Ⱦ6���棬�����ܸ����������Ҽ���draw call����
		// ���������Ļ�����Ҫ�ü���shader���ֵ�ǰ���ĸ��棬���Ҽ���shader������������ܿ���

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tbo_EnvCubemap, 0);

		// ���֡�������������
		int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error: Framebuffer is not complete!" << endl;
			cout << "Check Flag: " << hex << chkFlag << endl;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.cube_env.DrawMesh(scene.GetEquireColorShader, GL_TRIANGLES);
	}

	// ע�⣬����glGenerateMipmap�����ɶ�Ӧ��mipmap�ڴ沢�������ɶ�Ӧ��mipmap�������ʱ��û�л���ԭͼ��ֻ�������ɫ
	// ��Ϊ����ֻ��Ҫ�Զ����ɵ�mipmap, ��� �����ڻ��ƻ���cubemap֮���ٵ���glGenerateMipmap���������ڷ��仺���ʱ�����
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo_EnvCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SkyBoxTest(Mesh& skybox)
{
	// ��ո���������
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// GL_BLEND enableʱ����������û��aplhaͨ�������¿��������壬����Ҫ�رա�
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	skybox.DrawMesh(scene.cubemapShader, GL_TRIANGLES);
}

//�����Զ���֡���� IBL ����� Cubemap
void CreateFrameBuffer_IrdCubemap()
{
	glGenFramebuffers(1, &fbo_irdCubemap);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_irdCubemap);

	// ���������� ��Ӧdepth����
	glGenTextures(1, &tbo_irdCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo_irdCubemap);
	for (uint i = 0; i < 6; i++)
	{
		// HDR����������Ӧ���� GL_FLOAT
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, IRRADIANCE_RESOLUTION, IRRADIANCE_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// color�����ȵ���Ⱦ��ʱ���ٹ���

	// depth��������Ҫ

	// ���֡�������������
	// ��ΪҪ�ȵ���Ⱦ��ʱ����������������Դ˴����ж���������

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ���Ʒ����Cubemap 
void DrawIrradianceCubemap()
{
	// projection
	float aspect = 1.0f;
	float near = 0.1f; // �����׶�Ľ�ƽ�����ù��󣬻ᵼ�²����ƽ�����Ƭ��
	float far = 10.0f;
	mat4 projection = perspective(radians(90.0f), aspect, near, far);

	// view
	vector<mat4> transforms;
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));

	// Set Uniform To Shader
	scene.irradianceShader.Use();

	glViewport(0, 0, IRRADIANCE_RESOLUTION, IRRADIANCE_RESOLUTION);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_irdCubemap);

	for (uint i = 0; i < 6; i++)
	{
		scene.irradianceShader.SetMat4("transforms", transforms[i]);

		// ������ÿdrawһ���棬����һ�����color attachment
		// Ҳ����һ��draw����shader��ֱ���Ⱦ6���棬�����ܸ����������Ҽ���draw call����
		// ���������Ļ�����Ҫ�ü���shader���ֵ�ǰ���ĸ��棬���Ҽ���shader������������ܿ���

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tbo_irdCubemap, 0);

		// ���֡�������������
		int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error: Framebuffer is not complete!" << endl;
			cout << "Check Flag: " << hex << chkFlag << endl;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		scene.cube_irradiance.DrawMesh(scene.irradianceShader, GL_TRIANGLES);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//�����Զ���֡���� IBL pre filtered Cubemap
void CreateFrameBuffer_PrefilterCubemap()
{
	glGenFramebuffers(1, &fbo_pfCubemap);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_pfCubemap);

	// ���������� ��Ӧdepth����
	glGenTextures(1, &tbo_pfCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tbo_pfCubemap);
	for (uint i = 0; i < 6; i++)
	{
		// HDR����������Ӧ���� GL_FLOAT
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, PREFILTERED_RESOLUTION, PREFILTERED_RESOLUTION, 0, GL_RGB, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // ������  u v mipmap
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				   // ˫����  u v
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// ��Ϊ����prefilter cubemap����Ҫʵ�ֲִڶȺ�miplevel�Ķ�Ӧ��ϵ��
	// ����ڻ���ԭͼ���ٵ���glGenerateMipmap�Զ�����mipmap����Ȼ�޷������������
	// ��ˣ�������(����ԭͼ֮ǰ)�ȵ���glGenerateMipmap�����Զ�����mipmap�ڴ棬��ʱmipmapΪȫ��(����ν)
	// �����ڻ���ԭͼ��ʱ���ֶ����ɶ�Ӧ��mipmap
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// color�����ȵ���Ⱦ��ʱ���ٹ���

	// depth��������Ҫ

	// ���֡�������������
	// ��ΪҪ�ȵ���Ⱦ��ʱ����������������Դ˴����ж���������

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ����PBR Specular Ԥ�˲�Cubemap 
void DrawPrefilterCubemap()
{
	// projection
	float aspect = 1.0f;
	float near = 0.1f; // �����׶�Ľ�ƽ�����ù��󣬻ᵼ�²����ƽ�����Ƭ��
	float far = 10.0f;
	mat4 projection = perspective(radians(90.0f), aspect, near, far);

	// view
	vector<mat4> transforms;
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	transforms.push_back(projection * lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));


	scene.prefilterShader.Use();

	glBindFramebuffer(GL_FRAMEBUFFER, fbo_pfCubemap);

	const uint maxMipLevel = 5;

	scene.prefilterShader.SetFloat("resolution", (float)ENVIRONMENT_RESOLUTION);

	for (uint mip = 0; mip < maxMipLevel; mip++)
	{
		float mipWidth = PREFILTERED_RESOLUTION * pow(0.5, mip);
		float mipHeight = PREFILTERED_RESOLUTION * pow(0.5, mip);

		glViewport(0, 0, mipWidth, mipHeight);

		// roughness 0.0 0.25 0.5 0.75 1.0 
		float roughness = (float)mip / (float)(maxMipLevel - 1);
		scene.prefilterShader.SetFloat("roughness", roughness);

		for (uint i = 0; i < 6; i++)
		{
			scene.prefilterShader.SetMat4("transforms", transforms[i]);

			// ������ÿdrawһ���棬����һ�����color attachment
			// Ҳ����һ��draw����shader��ֱ���Ⱦ6���棬�����ܸ����������Ҽ���draw call����
			// ���������Ļ�����Ҫ�ü���shader���ֵ�ǰ���ĸ��棬���Ҽ���shader������������ܿ���

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tbo_pfCubemap, mip);

			// ���֡�������������
			int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
			{
				cout << "Error: Framebuffer is not complete!" << endl;
				cout << "Check Flag: " << hex << chkFlag << endl;
			}

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			scene.cube_irradiance.DrawMesh(scene.prefilterShader, GL_TRIANGLES);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ����PBR��BRDF������
void CreateFrameBuffer_BRDF()
{
	glGenFramebuffers(1, &fbo_BRDF);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_BRDF);

	/**********************color���壬����F0�ı�����ƫ��*********************/
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo_BRDF);

	// �����������
	glBindTexture(GL_TEXTURE_2D, tbo_BRDF);
	// BRDF��˫ͨ��float����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, BRDF_RESOLUTION, BRDF_RESOLUTION, 0, GL_RG, GL_FLOAT, NULL); 
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo_BRDF, 0);

	// ����Ҫrender buffer object

	// ���֡�������������
	int chkFlag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (chkFlag != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
		cout << "Check Flag: " << hex << chkFlag << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawBRDF()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_BRDF);

	glDisable(GL_DEPTH_TEST);

	// ��ո���������
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, BRDF_RESOLUTION, BRDF_RESOLUTION);

	// BRDFShader����Ҫ����
	scene.BRDFScreen.DrawMesh(scene.BRDFShader, GL_TRIANGLES);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}