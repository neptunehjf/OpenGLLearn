#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "common.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include <random>

class Scene
{
public:
	Scene()
	{
	}
	
	Shader lightShader;
	Shader screenShader;
	Shader cubemapShader;
	Shader reflectShader;
	Shader refractShader;
	Shader normalShader;
	Shader lightInstShader;
	Shader depthmapShader;
	Shader depthmapDisplayShader;
	Shader depthCubemapShader;
	Shader GBufferShader;
	Shader DeferredShader;
	Shader ForwardShader;
	Shader DeferredLampShader;
	Shader GBufferSSAOShader;
	Shader SSAOShader;
	Shader SSAOBlurShader;
	Shader PBRShader;
	Shader PBRWithTextureShader;
	Shader GetEquireColorShader;
	Shader irradianceShader; // 求采样值得辐射度

	Mesh cubeCubemap;
	Mesh cube;
	Mesh skybox;
	Mesh square;
	Mesh plane;
	Mesh screen;
	Mesh mirror;
	Mesh particle; 
	Model nanosuit;
	Model planet;
	Model rock;
	Mesh lamp;
	Mesh PosYSquare;
	Mesh defferedScreen;
	Mesh SSAOScreen;
	Mesh SSAOBlurScreen;
	Mesh sphere;
	Mesh cube_env;
	Mesh skybox_env;
	Mesh cube_irradiance;
	Mesh skybox_irradiance;

	vector<vec3> lightPositions;
	vector<vec3> lightColors;
	vector<float> lightRadius;

	vector<vec3> squarePositions;
	vector<mat4> instMat4;

	vector<Texture> m_brickTexture;

	Camera* myCam;

	// SSAO采样kernel
	vector<vec3> ssaoKernel;
	// SSAO采样Noise
	vector<vec3> ssaoNoise;
	GLuint noiseTexture;

	void CreateScene(Camera* myCam);
	void DrawScene(bool bDepthmap = false, bool bDepthCubemap = false, bool bGBuffer = false);
	bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t);
	bool LoadHDRTexture(const string&& filePath, GLuint& texture);
	GLuint LoadCubemap(const vector<string>& cubemapFaces);
	void DeleteScene();
	void CreateShader();
	void UpdateNMVertices();
	void DrawScene_DeferredTest();
	void DrawScene_SSAOTest();
	void DrawScene_PBR();

private:
	void CreateAsteroid();
	void CreateNMVertices(vector<VertexNM>& verticesNM);
	void CalcTangent(vector<VertexNM>& vertices, vec3& tangent, vec3& bitangent);
	void CreateLightsInfo();
	void CreateSSAOSamples();
	GLfloat lerp(GLfloat a, GLfloat b, GLfloat f);
	void CreateSSAONoise();
	void CreateSSAONoiseTexture();
	Mesh CreateSphereMesh(const vector<Texture>& texture);
};

void Scene::CreateShader()
{
	// 创建shader 不能声明全局变量，因为shader的相关操作必须在glfw初始化完成后
	lightShader = Shader("Lighting.vs", "Lighting.fs", "Lighting.gs");
	screenShader = Shader("PostProcess.vs", "PostProcess.fs");
	cubemapShader = Shader("Cubemap.vs", "Cubemap.fs");
	reflectShader = Shader("Reflection.vs", "Reflection.fs");
	refractShader = Shader("Refraction.vs", "Refraction.fs");
	normalShader = Shader("Normal.vs", "Normal.fs", "Normal.gs");
	lightInstShader = Shader("LightingInstance.vs", "LightingInstance.fs");
	depthmapShader = Shader("DepthMap.vs", "DepthMap.fs");
	depthmapDisplayShader = Shader("DepthmapDisplay.vs", "DepthmapDisplay.fs");
	depthCubemapShader = Shader("DepthCubemap.vs", "DepthCubemap.fs", "DepthCubemap.gs");
	GBufferShader = Shader("G-buffer.vs", "G-buffer.fs");
	DeferredShader = Shader("DeferredShading.vs", "DeferredShading.fs");
	ForwardShader = Shader("ForwardShading.vs", "ForwardShading.fs");
	DeferredLampShader = Shader("DeferredLamp.vs", "DeferredLamp.fs");
	GBufferSSAOShader = Shader("G-buffer-SSAO.vs", "G-buffer-SSAO.fs");
	SSAOShader = Shader("SSAO.vs", "SSAO.fs");
	SSAOBlurShader = Shader("SSAO_Blur.vs", "SSAO_Blur.fs");
	PBRShader = Shader("PBR.vs", "PBR.fs");
	PBRWithTextureShader = Shader("PBRWithTexture.vs", "PBRWithTexture.fs");
	GetEquireColorShader = Shader("GetEquireColor.vs", "GetEquireColor.fs");
	irradianceShader = Shader("GetEquireColor.vs", "Irradiance.fs");
}

void Scene::CreateScene(Camera* myCam)
{
	this->myCam = myCam;
	/* 加载贴图 */

	// 加载贴图
	GLuint t_metal = 0;
	GLuint t_marble = 0;
	GLuint t_window = 0;
	GLuint t_wood = 0;
	GLuint t_white = 0;
	GLuint t_brick = 0;
	GLuint t_brick_normal = 0;
	GLuint t_brick2 = 0;
	GLuint t_brick2_normal = 0;
	GLuint t_brick2_disp = 0;
	GLuint t_plastic_albedo = 0;
	GLuint t_rusted_iron_albedo = 0;
	GLuint t_rusted_iron_metallic = 0;
	GLuint t_rusted_iron_roughness = 0;
	GLuint t_rusted_iron_ao = 0;
	GLuint t_hdr_loft = 0;

	// 翻转y轴，使图片和opengl坐标一致  但是如果assimp 导入模型时设置了aiProcess_FlipUVs，就不能重复设置了
	stbi_set_flip_vertically_on_load(true);

	LoadTexture("Resource/Texture/metal.png", t_metal, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/marble.jpg", t_marble, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/window.png", t_window, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	LoadTexture("Resource/Texture/wood.png", t_wood, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/AllWhite.png", t_white, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/bricks2.jpg", t_brick2, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/bricks2_normal.jpg", t_brick2_normal, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/bricks2_disp.jpg", t_brick2_disp, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/pbr/plastic/albedo.png", t_plastic_albedo, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/pbr/rusted_iron/albedo.png", t_rusted_iron_albedo, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/pbr/rusted_iron/metallic.png", t_rusted_iron_metallic, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/pbr/rusted_iron/roughness.png", t_rusted_iron_roughness, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/pbr/rusted_iron/ao.png", t_rusted_iron_ao, GL_REPEAT, GL_REPEAT);
	LoadHDRTexture("Resource/Texture/hdr/newport_loft.hdr", t_hdr_loft);

	stbi_set_flip_vertically_on_load(false);
	LoadTexture("Resource/Texture/brickwall.jpg", t_brick, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/brickwall_normal.jpg", t_brick_normal, GL_REPEAT, GL_REPEAT);

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
		{t_wood, "texture_diffuse"},
		{t_wood, "texture_specular"}
	};

	const vector<Texture> cubeTexture =
	{
		{t_marble, "texture_diffuse"}
	};

	const vector<Texture> windowTexture =
	{
		{t_window, "texture_diffuse"}
	};

	const vector<Texture> skyboxTexture =
	{
		{t_cubemap, "texture_cubemap"}
	};

	const vector<Texture> lampTexture =
	{
		{t_white, "texture_diffuse"}
	};

	const vector<Texture> brickTexture =
	{
		{t_brick, "texture_diffuse"},
		{t_brick_normal, "texture_normal"}
	};

	const vector<Texture> brick2Texture =
	{
		{t_brick2, "texture_diffuse"},
		{t_brick2_normal, "texture_normal"},
		{t_brick2_disp, "texture_disp"}
	};

	const vector<Texture> plasticTexture =
	{
		{t_plastic_albedo, "texture_diffuse"}
	};

	const vector<Texture> rustyIronTexture =
	{
		{t_rusted_iron_albedo, "texture_diffuse"},
		{t_rusted_iron_metallic, "texture_diffuse"},
		{t_rusted_iron_roughness, "texture_diffuse"},
		{t_rusted_iron_ao, "texture_diffuse"}
	};

	const vector<Texture> hdrLoftTexture =
	{
		{t_hdr_loft, "texture_diffuse"}
	};


	// 深拷贝
	m_brickTexture = brick2Texture;

	vector<vec2> instanceArray;
	int index = 0;
	float offset = 0.1f;
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			instanceArray.push_back(translation);
		}
	}

	plane = Mesh(g_3DPlaneVertices, g_3DPlaneIndices, planeTexture);
	plane.SetScale(vec3(100.0f, 0.1f, 100.0f));
	plane.SetTranslate(vec3(0.0f, 1.0f, 0.0f));
	cubeCubemap = Mesh(g_cubeVertices, g_cubeIndices, skyboxTexture);
	cube = Mesh(g_cubeVertices, g_cubeIndices, lampTexture);
	square = Mesh(g_squareVertices, g_squareIndices, windowTexture);
	skybox = Mesh(g_skyboxVertices, g_skyboxIndices, skyboxTexture);
	screen = Mesh(g_screenVertices, g_screenIndices);
	defferedScreen = Mesh(g_screenVertices, g_screenIndices);
	SSAOScreen = Mesh(g_screenVertices, g_screenIndices);
	SSAOBlurScreen = Mesh(g_screenVertices, g_screenIndices);
	mirror = Mesh(g_mirrorVertices, g_mirrorIndices);
	particle = Mesh(g_particleVertices, g_particleIndices);
	lamp = Mesh(g_cubeVertices, g_cubeIndices, lampTexture);
	lamp.SetScale(vec3(1.0f));
	sphere = CreateSphereMesh(rustyIronTexture);
	cube_env = Mesh(g_cubeVertices, g_cubeIndices, hdrLoftTexture);

	if (bEnableNormalMap)
	{
		CreateNMVertices(g_planeVerticesNM);
		PosYSquare = Mesh(g_planeVerticesNM, brick2Texture);
	}
	else
	{
		PosYSquare = Mesh(g_planeVertices, g_planeIndices, brick2Texture);
	}
	
	squarePositions.push_back(glm::vec3(-1.5f, 1.0f, -0.48f));
	squarePositions.push_back(glm::vec3(1.5f, 1.0f, 0.51f));
	squarePositions.push_back(glm::vec3(0.0f, 1.0f, 0.7f));
	squarePositions.push_back(glm::vec3(-0.3f, 1.0f, -2.3f));
	squarePositions.push_back(glm::vec3(0.5f, 1.0f, -0.6f));

	nanosuit = Model("Resource/Model/nanosuit_reflection/nanosuit.obj");
	//vector<Mesh> suitMeshes = nanosuit.meshes;     // 赋值号，默认vector是深拷贝，因此SetTextures不会影响nanosuit对象
	//vector<Mesh>& suitMeshes = nanosuit.meshes;    // 使用引用，引用只是nanosuit.meshes的别名，因此SetTextures会影响到nanosuit对象
	vector<Mesh>& suitMeshes = nanosuit.GetMeshes(); // 使用引用，引用只是nanosuit.meshes的别名，因此SetTextures会影响到nanosuit对象
	for (unsigned int i = 0; i < suitMeshes.size(); i++)
	{
		suitMeshes[i].AddTextures(skyboxTexture);
	}

	planet = Model("Resource/Model/planet/planet.obj");
	planet.SetTranslate(vec3(40.0f, 40.0f, 40.0f));
	planet.SetScale(vec3(20.0f, 20.0f, 20.0f));

	CreateAsteroid();
	rock = Model("Resource/Model/rock/rock.obj", instMat4);

	CreateLightsInfo();
}

void Scene::DrawScene(bool bDepthmap, bool bDepthCubemap, bool bGBuffer)
{
	if (bBlending && !bGBuffer)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// 清空各个缓冲区
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //离屏渲染不需要glClear(GL_COLOR_BUFFER_BIT);

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	// 绘制地板  // 对地板不绘制深度图
	if (bDepthmap)
		;
	else if (bDepthCubemap)
		;
	else if (bGBuffer)
		plane.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
		plane.DrawMesh(lightShader, GL_TRIANGLES);

	// 绘制立方体
	cubeCubemap.SetTranslate(vec3(1.0f, 1.5f, 1.0f));
	if (bDepthmap)
		cubeCubemap.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		cubeCubemap.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else if (bGBuffer)
		cubeCubemap.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
	{
		cubeCubemap.DrawMesh(reflectShader, GL_TRIANGLES);
		cubeCubemap.DrawMesh(normalShader, GL_TRIANGLES);
	}

	cubeCubemap.SetTranslate(vec3(0.0f, 1.5f, -1.0f));
	if (bDepthmap)
		cubeCubemap.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		cubeCubemap.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else if (bGBuffer)
		cubeCubemap.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
	{
		cubeCubemap.DrawMesh(refractShader, GL_TRIANGLES);
		cubeCubemap.DrawMesh(normalShader, GL_TRIANGLES);
	}

	cube.SetTranslate(vec3(3.0f, 1.5f, 0.0f));
	if (bDepthmap)
		cube.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		cube.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else if (bGBuffer)
		cube.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
	{
		cube.DrawMesh(lightShader, GL_TRIANGLES);
		cube.DrawMesh(normalShader, GL_TRIANGLES);
	}

	glDisable(GL_BLEND);
	// 绘制人物
	nanosuit.SetScale(vec3(0.1f));
	nanosuit.SetTranslate(vec3(1.0f, 1.0f, 0.0f));
	if (bDepthmap)
		nanosuit.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		nanosuit.DrawModel(depthCubemapShader);
	else if (bGBuffer)
		nanosuit.DrawModel(GBufferShader);
	else
	{
		nanosuit.DrawModel(lightShader);
		nanosuit.DrawModel(normalShader);
	}

	nanosuit.SetTranslate(vec3(0.0f, 1.0f, -3.0f));
	if (bDepthmap)
		nanosuit.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		nanosuit.DrawModel(depthCubemapShader);
	else if (bGBuffer)
		nanosuit.DrawModel(GBufferShader);
	else
	{
		nanosuit.DrawModel(reflectShader);
	}
	nanosuit.SetTranslate(vec3(3.0f, 1.0f, -3.0f));
	if (bDepthmap)
		nanosuit.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		nanosuit.DrawModel(depthCubemapShader);
	else if (bGBuffer)
		nanosuit.DrawModel(GBufferShader);
	else
	{
		nanosuit.DrawModel(refractShader);
	}
	
	if (!bGBuffer)
		glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);
	if (bSkyBox)
	{
		// 绘制天空盒
		skybox.DrawMesh(cubemapShader, GL_TRIANGLES);
	}
	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	// 虽然每帧固定旋转一定角度很方便，但是会导致旋转速度受帧数影响，一般不用这种方法
	// planet.AddRotate(ROTATE_SPEED_PLANET * deltaTime, vec3(0.0f, 1.0f, 0.0f)); 
	
	// 一般用每秒固定旋转一定角度的方式，这样虽然帧率低的时候会卡，但是不会影响游戏逻辑
	planet.AddRotate(ROTATE_SPEED_PLANET * deltaTime, vec3(0.0f, 1.0f, 0.0f));
	if (bDepthmap)
		planet.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		planet.DrawModel(depthCubemapShader);
	else if (bGBuffer)
		planet.DrawModel(GBufferShader);
	else
	{
		planet.DrawModel(lightShader);
	}

	if (bDepthmap)
		rock.DrawModel(depthmapShader, true);
	else if (bDepthCubemap)
		rock.DrawModel(depthCubemapShader, true);
	else if (bGBuffer)
		rock.DrawModel(GBufferShader, true);
	else
		rock.DrawModel(lightInstShader, true);

	for (int i = 0; i < 4; i++)
	{
		lamp.SetTranslate(lampPos[i]);
		if (bDepthmap)
			lamp.DrawMesh(depthmapShader, GL_TRIANGLES);
		else if (bDepthCubemap)
			lamp.DrawMesh(depthCubemapShader, GL_TRIANGLES);
		else if (bGBuffer)
			lamp.DrawMesh(GBufferShader, GL_TRIANGLES);
		else
			lamp.DrawMesh(lightShader, GL_TRIANGLES);
	}

	lamp.SetTranslate(lampWithShadowPos);
	if (bDepthmap)
		lamp.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		lamp.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else if (bGBuffer)
		lamp.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
		lamp.DrawMesh(lightShader, GL_TRIANGLES);		

	PosYSquare.SetTranslate(vec3(6.0f, 5.0f, 6.0f));
	if (bDepthmap)
		PosYSquare.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		PosYSquare.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else if (bGBuffer)
		PosYSquare.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
	{
		PosYSquare.DrawMesh(lightShader, GL_TRIANGLES);
		PosYSquare.DrawMesh(normalShader, GL_TRIANGLES);
	}

	// 按窗户离摄像机间的距离排序，map默认是升序排序，也就是从近到远
	// 必须放在render loop里，因为摄像机是实时改变的
	map<float, vec3> sorted;
	for (int i = 0; i < squarePositions.size(); i++)
	{
		float distance = length(myCam->camPos - squarePositions[i]);
		sorted[distance] = squarePositions[i];
	}
	// 透明物体必须最后绘制，并且透明物体之间要从远到近绘制
	for (map<float, vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
	{
		square.SetTranslate(it->second);
		if (bDepthmap)
			square.DrawMesh(depthmapShader, GL_TRIANGLES);
		else if (bDepthCubemap)
			square.DrawMesh(depthCubemapShader, GL_TRIANGLES);
		else if (bGBuffer)
			square.DrawMesh(GBufferShader, GL_TRIANGLES);
		else
			square.DrawMesh(lightShader, GL_TRIANGLES);
	}

	/* 一些功能测试 */
	glEnable(GL_PROGRAM_POINT_SIZE);
	//绘制的图元是GL_POINTS。对应的是裁剪空间的归一化坐标（实际是在顶点着色器设定）
	glPointSize(pointSize);
	particle.DrawMesh(screenShader, GL_POINTS);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

bool Scene::LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t)
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

	GLenum informat = 0;
	GLenum format = 0;
	if (channel == 1)
	{
		informat = GL_RED;
		format = GL_RED;
	}
	else if (channel == 3)
	{
		informat = GL_SRGB; // 如果启用了gamma校正，则输出在线性空间，所以在读取贴图的时候要转为线性空间贴图，不然颜色会不对
							// 因为sRGB格式的贴图，是为了在屏幕上按sRGB可以正常显示的贴图，因此贴图本身是经过gamma校正的，因此是发白的，再经过显示器的gamma2.2(sRGB)处理后正好恢复正常颜色
							// 这种情况再在渲染程序中gamma校正，就相当于2次gamma校正，所以画面会发白
							// 因此需要指定GL_SRGB，将图片转为线性空间（去除图片本身的gamma校正）
							// 说白了，显示出来是正常的图片，实际上是经过gamma校正的发白图片，如果我们想自己靠渲染实现gamma校正的话，就不需要图片本身的gamma校正了
		format = GL_RGB;
	}
	else if (channel == 4)
	{
		informat = GL_SRGB_ALPHA;
		format = GL_RGBA;
	}

	if (!bGammaCorrection)
		informat = format;
		
	if (data)
	{
		// 贴图数据 内存 -> 显存
		glTexImage2D(GL_TEXTURE_2D, 0, informat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

bool Scene::LoadHDRTexture(const string&& filePath, GLuint& texture)
{
	int width, height, nrComponents;
	float* data = stbi_loadf(filePath.c_str(), &width, &height, &nrComponents, 0);

	if (data)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		return true;
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
		return false;
	}
}

GLuint Scene::LoadCubemap(const vector<string>& cubemapFaces)
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

		GLenum informat = 0;
		GLenum format = 0;
		if (channel == 1)
		{
			informat = GL_RED;
			format = GL_RED;
		}
		else if (channel == 3)
		{
			informat = GL_SRGB; //如果启用了gamma校正，则输出在线性空间，所以在读取贴图的时候要转为线性空间贴图，不然颜色会不对
			format = GL_RGB;
		}
		else if (channel == 4)
		{
			informat = GL_SRGB_ALPHA;
			format = GL_RGBA;
		}

		if (!bGammaCorrection)
			informat = format;

		if (data)
		{
			// 贴图数据 内存 -> 显存
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, informat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

void Scene::DeleteScene()
{
	nanosuit.DeleteModel();
	plane.DeleteMesh();
	cubeCubemap.DeleteMesh();
	cube.DeleteMesh();
	skybox.DeleteMesh();
	square.DeleteMesh();
	screen.DeleteMesh();
	lightShader.Remove();
	screenShader.Remove();
	cubemapShader.Remove();
}

void Scene::CreateAsteroid()
{
	srand(glfwGetTime()); // 初始化随机种子    
	float radius = 150.0;
	float offset = 30.0f;
	for (unsigned int i = 0; i < ROCK_NUM; i++)
	{
		mat4 model = mat4(1.0f);
		// 1. 位移：分布在半径为 'radius' 的圆形上，偏移的范围是 [-offset, offset]
		float angle = (float)i / (float)ROCK_NUM * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // 让行星带的高度比x和z的宽度要小
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = translate(model, vec3(x, y, z) + vec3(40.0f, 45.0f, 40.0f));

		// 2. 缩放：在 0.05 和 0.25f 之间缩放
		float _scale = (rand() % 20) / 100.0f + 0.05;
		model = scale(model, vec3(_scale));

		// 3. 旋转：绕着一个（半）随机选择的旋转轴向量进行随机的旋转
		float rotAngle = (rand() % 360);
		model = rotate(model, rotAngle, vec3(0.4f, 0.6f, 0.8f));

		// 4. 添加到矩阵的数组中
		instMat4.push_back(model);
	}
}

// 参数vertices的大小必须是三角形的3个顶点信息
void Scene::CalcTangent(vector<VertexNM>& vertices, vec3& tangent, vec3& bitangent)
{
	if (vertices.size() != 3)
	{
		cout << "CalcTangent Error: Vertices size is not 3!" << endl;
		return;
	}

	// 取出顶点和UV信息
	vec3 pos1 = vertices[0].position;
	vec3 pos2 = vertices[1].position;
	vec3 pos3 = vertices[2].position;
	vec2 uv1 = vertices[0].texCoord;
	vec2 uv2 = vertices[1].texCoord;
	vec2 uv3 = vertices[2].texCoord;

	// 1 计算 edge 和 deltaUV
	vec3 edge1 = pos2 - pos1;
	vec3 edge2 = pos3 - pos1;
	vec2 deltaUV1 = uv2 - uv1;
	vec2 deltaUV2 = uv3 - uv1;

	// 2 计算Tangent 和 bitangent
	GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent = normalize(tangent);

	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent = normalize(bitangent);
}

void Scene::CreateNMVertices(vector<VertexNM>& verticesNM)
{
	for (uint i = 0; i < verticesNM.size(); i += 3)
	{
		// 计算切线
		vector<VertexNM> temp_vertices;
		vec3 tangent, bitangent;
		temp_vertices.push_back(verticesNM[i]);
		temp_vertices.push_back(verticesNM[i + 1]);
		temp_vertices.push_back(verticesNM[i + 2]);
		CalcTangent(temp_vertices, tangent, bitangent);

		// 计算结果存入顶点数据
		verticesNM[i].tangent = tangent;
		verticesNM[i].bitangent = bitangent;
		verticesNM[i + 1].tangent = tangent;
		verticesNM[i + 1].bitangent = bitangent;
		verticesNM[i + 2].tangent = tangent;
		verticesNM[i + 2].bitangent = bitangent;
	}
}

void Scene::UpdateNMVertices()
{
	if (bEnableNormalMap)
	{
		CreateNMVertices(g_planeVerticesNM);
		PosYSquare = Mesh(g_planeVerticesNM, m_brickTexture);
	}
	else
	{
		PosYSquare = Mesh(g_planeVertices, g_planeIndices, m_brickTexture);
	}
}

// 用于测试deferred shading对GPU的渲染效率，因此只渲染少量Mesh，减少CPU对测试的影响
void Scene::DrawScene_DeferredTest()
{
	// 清空各个缓冲区
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //离屏渲染不需要glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);

	// 如果想要把深度图写入rbo，必须打开深度测试。
	glEnable(GL_DEPTH_TEST);

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	plane.SetScale(vec3(100.0f, 0.1f, 100.0f));
	plane.SetTranslate(vec3(0.0f, 0.0f, 0.0f));
	if (bDeferred)
		plane.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
		plane.DrawMesh(ForwardShader, GL_TRIANGLES);

	nanosuit.SetScale(vec3(0.1f));
	nanosuit.SetRotate(-190.0f, vec3(1.0f, 0.0f, 0.0f));
	nanosuit.SetTranslate(vec3(0.0f, 0.1f, 0.0f));
	if (bDeferred)
		nanosuit.DrawModel(GBufferShader);
	else
		nanosuit.DrawModel(ForwardShader);

	// 与Combined Shading做对照
	for (uint i = 0; i < HEAVY_LIGHTS_NUM; i++)
	{
		cube.SetScale(vec3(0.1f));
		cube.SetTranslate(lightPositions[i]);
		if (bDeferred)
			cube.DrawMesh(GBufferShader, GL_TRIANGLES);
		else
			cube.DrawMesh(ForwardShader, GL_TRIANGLES);
	}
}

// 创建大量灯源，用于deferred shading测试
void Scene::CreateLightsInfo()
{
	srand(13);
	for (uint i = 0; i < HEAVY_LIGHTS_NUM; i++)
	{
		// calculate slightly random offsets
		float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
		float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
		float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
		lightPositions.push_back(vec3(xPos, yPos, zPos));
		// also calculate random color
		float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
		lightColors.push_back(vec3(rColor, gColor, bColor));

		// calculate light radius
		GLfloat constant = 1.0;
		GLfloat linear = 0.7;
		GLfloat quadratic = 1.8;
		GLfloat lightMax = std::fmaxf(std::fmaxf(rColor, gColor), bColor);
		GLfloat radius =
			(-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
			/ (2 * quadratic);
		lightRadius.push_back(radius);
	}
}

void Scene::DrawScene_SSAOTest()
{
	// 清空各个缓冲区
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// GL_BLEND enable时，可能由于没有aplha通道，导致看不见物体，所以要关闭。
	glDisable(GL_BLEND);

	// 如果想要把深度图写入rbo，必须打开深度测试。但是这里不加也行，因为在FS手动把深度写入了color附件
	glEnable(GL_DEPTH_TEST);

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	plane.SetScale(vec3(100.0f, 0.1f, 100.0f));
	plane.SetTranslate(vec3(0.0f, 0.0f, 0.0f));
	plane.DrawMesh(GBufferSSAOShader, GL_TRIANGLES);

	nanosuit.SetScale(vec3(0.1f));
	nanosuit.SetRotate(-190.0f, vec3(1.0f, 0.0f, 0.0f));
	nanosuit.SetTranslate(vec3(0.0f, 0.1f, 0.0f));
	nanosuit.DrawModel(GBufferSSAOShader);


	CreateSSAOSamples();
	CreateSSAONoise();
	CreateSSAONoiseTexture();
}

// 创建SSAO采样点
void Scene::CreateSSAOSamples()
{
	uniform_real_distribution<GLfloat> randomFloats(0.0f, 1.0f); // 随机浮点数，范围0.0 - 1.0
	default_random_engine generator;

	for (uint i = 0; i < iSSAOSampleNum; i++)
	{
		// 以下坐标基于Tangent空间
		vec3 sample(randomFloats(generator) * 2.0f - 1.0f, // x分量在 -1.0到1.0之间随机
					randomFloats(generator) * 2.0f - 1.0f, // y分量在 -1.0到1.0之间随机
					randomFloats(generator));              // z分量在 0到1.0之间随机
		// 转为方向向量
		sample = normalize(sample);
		// 长度随机
		sample *= randomFloats(generator);

		// i越小采样点离片段越近
		GLfloat scale = (GLfloat)i / (GLfloat)iSSAOSampleNum;

		// 原函数是自变量是一次项的情况，是线性函数，但是这里自变量是二次项，所以视为i的二次函数
		// 曲线特征是大部分y值都集中在较小的地方，因此比较适合在离片段较近的地方采样
		scale = lerp(0.1f, 1.0f, scale * scale);
		// 采样点分布调整为主要集中在片段附近
		sample *= scale;

		ssaoKernel.push_back(sample);
	}
}

// 返回a到b之间的自变量为f的线性插值
GLfloat Scene::lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}

// 用绕z轴旋转采样点的方式，创建SSAO噪声
void Scene::CreateSSAONoise()
{
	uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
	default_random_engine generator;

	// iSSAONoise * iSSAONoise的正方形
	uint num = iSSAONoise * iSSAONoise;

	for (uint i = 0; i < num; i++)
	{
		// 以下坐标基于Tangent空间
		vec3 noise(randomFloats(generator) * 2.0f - 1.0f, // x分量在 -1.0到1.0之间随机
				   randomFloats(generator) * 2.0f - 1.0f, // y分量在 -1.0到1.0之间随机
				   0.0f);                                 // z分量保持0，因为是基于z轴旋转
		ssaoNoise.push_back(noise);
	}
}

void Scene::CreateSSAONoiseTexture()
{
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	// Noise数据写入图片
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, iSSAONoise, iSSAONoise, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// 因为要把Noise Texture平铺在屏幕上，所以要用GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Mesh Scene::CreateSphereMesh(const vector<Texture>& texture)
{
	vector<vec3> positions;
	vector<vec2> uv;
	vector<vec3> normals;
	vector<unsigned int> indices;

	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;
	const float PI = 3.14159265359f;
	for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
	{
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			positions.push_back(vec3(xPos, yPos, zPos));
			uv.push_back(vec2(xSegment, ySegment));
			normals.push_back(vec3(xPos, yPos, zPos));
		}
	}

	// Q: 这里为什么要分奇偶来得到下标有点没搞懂，以后有时间研究一下吧
	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				indices.push_back(y * (X_SEGMENTS + 1) + x);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				indices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	vector<float> data;
	for (unsigned int i = 0; i < positions.size(); ++i)
	{
		data.push_back(positions[i].x);
		data.push_back(positions[i].y);
		data.push_back(positions[i].z);
		if (normals.size() > 0)
		{
			data.push_back(normals[i].x);
			data.push_back(normals[i].y);
			data.push_back(normals[i].z);
		}
		if (uv.size() > 0)
		{
			data.push_back(uv[i].x);
			data.push_back(uv[i].y);
		}
	}

	//因为Vertex设置了#pragma pack(1)，所以内存是1字节对齐的，所以可以直接把vector<float>转成vector<Vertex>来用
	return Mesh(*((vector<Vertex> *)&data), indices, texture);
}

void Scene::DrawScene_PBR()
{
	// 清空各个缓冲区
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// GL_BLEND enable时，可能由于没有aplha通道，导致看不见物体，所以要关闭。
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	sphere.SetScale(vec3(0.1f));
	sphere.DrawMesh(PBRWithTextureShader, GL_TRIANGLE_STRIP);

	// 用等距柱状投影对应位置的颜色来渲染到cube
	// cube_env.DrawMesh(GetEquireColorShader, GL_TRIANGLES);

}