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
	Shader irradianceShader; // �����ֵ�÷����

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

	// SSAO����kernel
	vector<vec3> ssaoKernel;
	// SSAO����Noise
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
	// ����shader ��������ȫ�ֱ�������Ϊshader����ز���������glfw��ʼ����ɺ�
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
	/* ������ͼ */

	// ������ͼ
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

	// ��תy�ᣬʹͼƬ��opengl����һ��  �������assimp ����ģ��ʱ������aiProcess_FlipUVs���Ͳ����ظ�������
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


	// ���
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
	//vector<Mesh> suitMeshes = nanosuit.meshes;     // ��ֵ�ţ�Ĭ��vector����������SetTextures����Ӱ��nanosuit����
	//vector<Mesh>& suitMeshes = nanosuit.meshes;    // ʹ�����ã�����ֻ��nanosuit.meshes�ı��������SetTextures��Ӱ�쵽nanosuit����
	vector<Mesh>& suitMeshes = nanosuit.GetMeshes(); // ʹ�����ã�����ֻ��nanosuit.meshes�ı��������SetTextures��Ӱ�쵽nanosuit����
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

	// ��ո���������
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //������Ⱦ����ҪglClear(GL_COLOR_BUFFER_BIT);

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	// ���Ƶذ�  // �Եذ岻�������ͼ
	if (bDepthmap)
		;
	else if (bDepthCubemap)
		;
	else if (bGBuffer)
		plane.DrawMesh(GBufferShader, GL_TRIANGLES);
	else
		plane.DrawMesh(lightShader, GL_TRIANGLES);

	// ����������
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
	// ��������
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
		// ������պ�
		skybox.DrawMesh(cubemapShader, GL_TRIANGLES);
	}
	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	// ��Ȼÿ֡�̶���תһ���ǶȺܷ��㣬���ǻᵼ����ת�ٶ���֡��Ӱ�죬һ�㲻�����ַ���
	// planet.AddRotate(ROTATE_SPEED_PLANET * deltaTime, vec3(0.0f, 1.0f, 0.0f)); 
	
	// һ����ÿ��̶���תһ���Ƕȵķ�ʽ��������Ȼ֡�ʵ͵�ʱ��Ῠ�����ǲ���Ӱ����Ϸ�߼�
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

	// ���������������ľ�������mapĬ������������Ҳ���Ǵӽ���Զ
	// �������render loop���Ϊ�������ʵʱ�ı��
	map<float, vec3> sorted;
	for (int i = 0; i < squarePositions.size(); i++)
	{
		float distance = length(myCam->camPos - squarePositions[i]);
		sorted[distance] = squarePositions[i];
	}
	// ͸��������������ƣ�����͸������֮��Ҫ��Զ��������
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

	/* һЩ���ܲ��� */
	glEnable(GL_PROGRAM_POINT_SIZE);
	//���Ƶ�ͼԪ��GL_POINTS����Ӧ���ǲü��ռ�Ĺ�һ�����꣨ʵ�����ڶ�����ɫ���趨��
	glPointSize(pointSize);
	particle.DrawMesh(screenShader, GL_POINTS);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

bool Scene::LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t)
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

	GLenum informat = 0;
	GLenum format = 0;
	if (channel == 1)
	{
		informat = GL_RED;
		format = GL_RED;
	}
	else if (channel == 3)
	{
		informat = GL_SRGB; // ���������gammaУ��������������Կռ䣬�����ڶ�ȡ��ͼ��ʱ��ҪתΪ���Կռ���ͼ����Ȼ��ɫ�᲻��
							// ��ΪsRGB��ʽ����ͼ����Ϊ������Ļ�ϰ�sRGB����������ʾ����ͼ�������ͼ�����Ǿ���gammaУ���ģ�����Ƿ��׵ģ��پ�����ʾ����gamma2.2(sRGB)��������ûָ�������ɫ
							// �������������Ⱦ������gammaУ�������൱��2��gammaУ�������Ի���ᷢ��
							// �����Ҫָ��GL_SRGB����ͼƬתΪ���Կռ䣨ȥ��ͼƬ�����gammaУ����
							// ˵���ˣ���ʾ������������ͼƬ��ʵ�����Ǿ���gammaУ���ķ���ͼƬ������������Լ�����Ⱦʵ��gammaУ���Ļ����Ͳ���ҪͼƬ�����gammaУ����
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
		// ��ͼ���� �ڴ� -> �Դ�
		glTexImage2D(GL_TEXTURE_2D, 0, informat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

		GLenum informat = 0;
		GLenum format = 0;
		if (channel == 1)
		{
			informat = GL_RED;
			format = GL_RED;
		}
		else if (channel == 3)
		{
			informat = GL_SRGB; //���������gammaУ��������������Կռ䣬�����ڶ�ȡ��ͼ��ʱ��ҪתΪ���Կռ���ͼ����Ȼ��ɫ�᲻��
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
			// ��ͼ���� �ڴ� -> �Դ�
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, informat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
	srand(glfwGetTime()); // ��ʼ���������    
	float radius = 150.0;
	float offset = 30.0f;
	for (unsigned int i = 0; i < ROCK_NUM; i++)
	{
		mat4 model = mat4(1.0f);
		// 1. λ�ƣ��ֲ��ڰ뾶Ϊ 'radius' ��Բ���ϣ�ƫ�Ƶķ�Χ�� [-offset, offset]
		float angle = (float)i / (float)ROCK_NUM * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // �����Ǵ��ĸ߶ȱ�x��z�Ŀ��ҪС
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = translate(model, vec3(x, y, z) + vec3(40.0f, 45.0f, 40.0f));

		// 2. ���ţ��� 0.05 �� 0.25f ֮������
		float _scale = (rand() % 20) / 100.0f + 0.05;
		model = scale(model, vec3(_scale));

		// 3. ��ת������һ�����룩���ѡ�����ת�����������������ת
		float rotAngle = (rand() % 360);
		model = rotate(model, rotAngle, vec3(0.4f, 0.6f, 0.8f));

		// 4. ��ӵ������������
		instMat4.push_back(model);
	}
}

// ����vertices�Ĵ�С�����������ε�3��������Ϣ
void Scene::CalcTangent(vector<VertexNM>& vertices, vec3& tangent, vec3& bitangent)
{
	if (vertices.size() != 3)
	{
		cout << "CalcTangent Error: Vertices size is not 3!" << endl;
		return;
	}

	// ȡ�������UV��Ϣ
	vec3 pos1 = vertices[0].position;
	vec3 pos2 = vertices[1].position;
	vec3 pos3 = vertices[2].position;
	vec2 uv1 = vertices[0].texCoord;
	vec2 uv2 = vertices[1].texCoord;
	vec2 uv3 = vertices[2].texCoord;

	// 1 ���� edge �� deltaUV
	vec3 edge1 = pos2 - pos1;
	vec3 edge2 = pos3 - pos1;
	vec2 deltaUV1 = uv2 - uv1;
	vec2 deltaUV2 = uv3 - uv1;

	// 2 ����Tangent �� bitangent
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
		// ��������
		vector<VertexNM> temp_vertices;
		vec3 tangent, bitangent;
		temp_vertices.push_back(verticesNM[i]);
		temp_vertices.push_back(verticesNM[i + 1]);
		temp_vertices.push_back(verticesNM[i + 2]);
		CalcTangent(temp_vertices, tangent, bitangent);

		// ���������붥������
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

// ���ڲ���deferred shading��GPU����ȾЧ�ʣ����ֻ��Ⱦ����Mesh������CPU�Բ��Ե�Ӱ��
void Scene::DrawScene_DeferredTest()
{
	// ��ո���������
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //������Ⱦ����ҪglClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);

	// �����Ҫ�����ͼд��rbo���������Ȳ��ԡ�
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

	// ��Combined Shading������
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

// ����������Դ������deferred shading����
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
	// ��ո���������
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// GL_BLEND enableʱ����������û��aplhaͨ�������¿��������壬����Ҫ�رա�
	glDisable(GL_BLEND);

	// �����Ҫ�����ͼд��rbo���������Ȳ��ԡ��������ﲻ��Ҳ�У���Ϊ��FS�ֶ������д����color����
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

// ����SSAO������
void Scene::CreateSSAOSamples()
{
	uniform_real_distribution<GLfloat> randomFloats(0.0f, 1.0f); // �������������Χ0.0 - 1.0
	default_random_engine generator;

	for (uint i = 0; i < iSSAOSampleNum; i++)
	{
		// �����������Tangent�ռ�
		vec3 sample(randomFloats(generator) * 2.0f - 1.0f, // x������ -1.0��1.0֮�����
					randomFloats(generator) * 2.0f - 1.0f, // y������ -1.0��1.0֮�����
					randomFloats(generator));              // z������ 0��1.0֮�����
		// תΪ��������
		sample = normalize(sample);
		// �������
		sample *= randomFloats(generator);

		// iԽС��������Ƭ��Խ��
		GLfloat scale = (GLfloat)i / (GLfloat)iSSAOSampleNum;

		// ԭ�������Ա�����һ���������������Ժ��������������Ա����Ƕ����������Ϊi�Ķ��κ���
		// ���������Ǵ󲿷�yֵ�������ڽ�С�ĵط�����˱Ƚ��ʺ�����Ƭ�νϽ��ĵط�����
		scale = lerp(0.1f, 1.0f, scale * scale);
		// ������ֲ�����Ϊ��Ҫ������Ƭ�θ���
		sample *= scale;

		ssaoKernel.push_back(sample);
	}
}

// ����a��b֮����Ա���Ϊf�����Բ�ֵ
GLfloat Scene::lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}

// ����z����ת������ķ�ʽ������SSAO����
void Scene::CreateSSAONoise()
{
	uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // �������������Χ0.0 - 1.0
	default_random_engine generator;

	// iSSAONoise * iSSAONoise��������
	uint num = iSSAONoise * iSSAONoise;

	for (uint i = 0; i < num; i++)
	{
		// �����������Tangent�ռ�
		vec3 noise(randomFloats(generator) * 2.0f - 1.0f, // x������ -1.0��1.0֮�����
				   randomFloats(generator) * 2.0f - 1.0f, // y������ -1.0��1.0֮�����
				   0.0f);                                 // z��������0����Ϊ�ǻ���z����ת
		ssaoNoise.push_back(noise);
	}
}

void Scene::CreateSSAONoiseTexture()
{
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	// Noise����д��ͼƬ
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, iSSAONoise, iSSAONoise, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// ��ΪҪ��Noise Textureƽ������Ļ�ϣ�����Ҫ��GL_REPEAT
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

	// Q: ����ΪʲôҪ����ż���õ��±��е�û�㶮���Ժ���ʱ���о�һ�°�
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

	//��ΪVertex������#pragma pack(1)�������ڴ���1�ֽڶ���ģ����Կ���ֱ�Ӱ�vector<float>ת��vector<Vertex>����
	return Mesh(*((vector<Vertex> *)&data), indices, texture);
}

void Scene::DrawScene_PBR()
{
	// ��ո���������
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// GL_BLEND enableʱ����������û��aplhaͨ�������¿��������壬����Ҫ�رա�
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	sphere.SetScale(vec3(0.1f));
	sphere.DrawMesh(PBRWithTextureShader, GL_TRIANGLE_STRIP);

	// �õȾ���״ͶӰ��Ӧλ�õ���ɫ����Ⱦ��cube
	// cube_env.DrawMesh(GetEquireColorShader, GL_TRIANGLES);

}