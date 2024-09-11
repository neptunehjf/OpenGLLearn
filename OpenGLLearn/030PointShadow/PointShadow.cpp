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
void CreateFrameBuffer_Depthmap(GLuint& fbo, GLuint& tbo);
void SetUniformBuffer();

Camera myCam(vec3(-12.67f, 15.99f, -4.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
GLFWwindow* window = NULL;

// ԭ��������
GLuint fbo1 = 0; // �Զ���֡�������
GLuint tbo1 = 0; // ��������󣨸�����
GLuint rbo1 = 0; // ��Ⱦ������󣨸�����

// ���Ӿ�����
GLuint fbo2 = 0; // �Զ���֡�������
GLuint tbo2 = 0; // ��������󣨸�����
GLuint rbo2 = 0; // ��Ⱦ������󣨸�����

// �м仺��
GLuint fbo3 = 0; // �Զ���֡�������
GLuint tbo3 = 0; // ��������󣨸�����
GLuint rbo3 = 0; // ��Ⱦ������󣨸�����

// ��Ȼ���
GLuint fbo_depthmap = 0; // �Զ���֡�������
GLuint tbo_depthmap = 0; // ��������󣨸�����

// Uniform����
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
	scene.CreateShader();
	//scene.CreateScene(&myCam);

	// ԭ��������
	CreateFrameBuffer_MSAA(fbo1, tbo1, rbo1);
	// ���Ӿ�����
	CreateFrameBuffer(fbo2, tbo2, rbo2);
	// �м仺��
	CreateFrameBuffer(fbo3, tbo3, rbo3);
	// depthmap����
	CreateFrameBuffer_Depthmap(fbo_depthmap, tbo_depthmap);

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

			glViewport(0, 0, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_depthmap);

			if (bFrontFaceCulling)
			{
				glCullFace(GL_FRONT); //��Ϊshadow acneֻ����һ��������ͬʱ�������۾��͹��߿���������²ſ��ܷ��������ֱ�ӽ����߿����ı���������������ȥ���Ϳ��Դӱ�����Ԥ��shadow acne��
				scene.DrawScene(true);
				glCullFace(GL_BACK);
			}
			else
			{
				scene.DrawScene(true);
			}

			scene.lightShader.Use();
			scene.lightShader.SetMat4("dirLightSpace", dirLightSpace);
			scene.lightShader.SetInt("shadowmap", 30);
			glActiveTexture(GL_TEXTURE30);
			glBindTexture(GL_TEXTURE_2D, fbo_depthmap);
		}

		// ԭ����
		glViewport(0, 0, windowWidth, windowHeight);
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

		// ���м�fbo�ķ�ʽʵ�֣�ʵ�����м�FBO����һ��ֻ��1�����������ͨ֡���塣��Blit������MSAA FBO���ƽ�ȥ��Ȼ��Ϳ������м�FBO��TBO�����ڴ����ˡ�
		// ȱ���ǣ�����ڴ˻����Ͻ��к���ȥ������ɫ������1�������������Ϊ����������ģ����ܻ����µ��¾��
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo3);

		// MSAA��������ʵҲ����ֱ�Ӵ�����ɫ�����в�������ΪMSAA�����ʽ����ͨ����һ�������Բ�������ͨ�������ֱ���á�
		// ������sampler2DMS�ķ�ʽ���룬���Ի�ȡ��ÿ�������㣬��Ҫ�����Զ��忹����㷨
		// ֻ�ǵ�����Ⱦ�����Ļ�����glBlitFramebuffer�͹���
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// ���Ӿ�����
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
		myCam.yawValue = myCam.yawValue - ((int)myCam.yawValue / 360) * 360;

		/********************** Ĭ��֡�������ǰ�����ʱд�� **********************/
		// �ص��Զ��建��Ķ�д�����л�����Ĭ�ϻ���
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		
		// ��ո���������
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); //������Ⱦ����ҪglClear(GL_COLOR_BUFFER_BIT);

		// ����Ļ
		GLuint t_dummy = 0;
		const vector<Texture> screenTexture =
		{
			{tbo3, "texture_diffuse"},
			{t_dummy, "texture_specular"}

		};
		scene.screen.SetTextures(screenTexture);
		scene.screen.DrawMesh(scene.screenShader, GL_TRIANGLES);

		// ���Ӿ�
		const vector<Texture> mirrorTexture =
		{
			{tbo2, "texture_diffuse"},
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

		glDisable(GL_FRAMEBUFFER_SRGB); //imgui���治��ҪgammaУ��
		// imgui��Ĭ�ϻ����л��ƣ���Ϊ�Ҳ���imguiҲ�к��ڴ���Ч��
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ���������� ��ѯ�¼�
		glfwSwapBuffers(window);
	}

	// ��Դ����
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

	// bugfix ���ڴ�С�仯��������Ĵ�СҲҪ��Ӧ�仯
	// ɾ���Ѵ��ڵĻ���
	glDeleteFramebuffers(1, &fbo1);
	glDeleteFramebuffers(1, &tbo1);
	glDeleteFramebuffers(1, &rbo1);
	glDeleteFramebuffers(1, &fbo2);
	glDeleteFramebuffers(1, &tbo2);
	glDeleteFramebuffers(1, &rbo2);
	glDeleteFramebuffers(1, &fbo3);
	glDeleteFramebuffers(1, &tbo3);
	glDeleteFramebuffers(1, &rbo3);
	// ԭ��������
	CreateFrameBuffer_MSAA(fbo1, tbo1, rbo1);
	// ���Ӿ�����
	CreateFrameBuffer(fbo2, tbo2, rbo2);
	// �м仺��
	CreateFrameBuffer(fbo3, tbo3, rbo3);

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

	if (ImGui::TreeNodeEx("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
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
		//ImGui::SliderFloat("pointLight position", &posValue, 0.0f, 10.0f);
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

	if (ImGui::TreeNodeEx("Test and Blend", ImGuiTreeNodeFlags_DefaultOpen))
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
		ImGui::SliderFloat("Explode Magnitude", &explodeMag, 0.0f, 5.0f);
		ImGui::SliderFloat("Normal Length", &normalLen, 0.0f, 0.8f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Instance", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("AntiAliasing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("MSAA", &bMSAA);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Gamma Correction", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Gamma Correction", &bGammaCorrection);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Shadow Mapping", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable Shadow", &bShadow);
		ImGui::Checkbox("Display Shadowmap", &bDisDepthmap);
		ImGui::Checkbox("Enable Bias", &bBias);
		ImGui::Checkbox("Enable Front Face Culling", &bFrontFaceCulling);

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
	shader.SetBool("bBias", bBias);

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
		shader.SetFloat(prefix + "constant", 1.0f);
		shader.SetFloat(prefix + "linear", linear);
		shader.SetFloat(prefix + "quadratic", quadratic);
	}
}

//�����Զ���֡����
void CreateFrameBuffer(GLuint& fbo, GLuint& tbo, GLuint& rbo)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT0���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
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

//�����Զ���֡���� depthmap
void CreateFrameBuffer_Depthmap(GLuint& fbo, GLuint& tbo)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// ���������� ��Ӧcolor����
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION_WIDTH, SHADOW_RESOLUTION_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ���������  ��Ϊһ��GL_COLOR_ATTACHMENT0���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tbo, 0);

	// ��ɫ����Ķ�д�����ΪGL_NONE��Ҳ���ǲ����д��ɫ����
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// ���֡�������������
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error: Framebuffer is not complete!" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//�����Զ���֡����MSAA
void CreateFrameBuffer_MSAA(GLuint& fbo, GLuint& tbo, GLuint& rbo)
{
	// ���ȴ���һ��֡������� ����color stencil depth��ɡ�Ĭ�ϻ�����Ҳ�С�ֻ��������Լ�����������������ʵ��һЩ����˼�Ĺ��ܣ�
	// ֻ��Ĭ�ϻ���������ͼ��(��Ϊ��GLFW���ڰ�)�����Խ��Ļ��岻������κ�ͼ����˿�������������Ⱦ
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// ����MSAA������ ��Ӧcolor����
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tbo);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLE_NUM, GL_RGB, windowWidth, windowHeight, true);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// MSAA���������  ��Ϊһ��GL_COLOR_ATTACHMENT0���� ���ӵ� ֡�������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tbo, 0);

	// ������Ⱦ������� ��Ӧstencil��depth����
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// stencil��depth��color����Ӧ�ö�Ҫ��ӦMSAA�����color�Ƕ�Ӧ��MSAA ��stencil��depthû�ж�ӦMSAA��stencil depth��color�ǲ�ƥ��ģ����´���
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLE_NUM, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); 
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
