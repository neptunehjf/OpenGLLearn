#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "common.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"

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
	Shader GMTestShader;
	Shader normalShader;
	Shader InstanceShader;
	Shader lightInstShader;

	Mesh cubeReflect;
	Mesh cubeMarble;
	Mesh skybox;
	Mesh square;
	Mesh plane;
	Mesh screen;
	Mesh mirror;
	Mesh particle; 
	Mesh GMTest;
	Model nanosuit;
	Mesh InstanceTest;
	Model planet;
	Model rock;

	vector<vec3> squarePositions;
	vector<mat4> instMat4;

	Camera* myCam;

	void CreateScene(Camera* myCam);
	void DrawScene();
	bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t);
	GLuint LoadCubemap(const vector<string>& cubemapFaces);
	void DeleteScene();
	void CreateAsteroid();
};

void Scene::CreateScene(Camera* myCam)
{
	this->myCam = myCam;

	// 创建shader 不能声明全局变量，因为shader的相关操作必须在glfw初始化完成后
	lightShader = Shader("ShaderLighting.vs", "ShaderLighting.fs", "ShaderLighting.gs");
	screenShader = Shader("ShaderPostProcess.vs", "ShaderPostProcess.fs");
	cubemapShader = Shader("ShaderCubemap.vs", "ShaderCubemap.fs");
	reflectShader = Shader("ShaderReflection.vs", "ShaderReflection.fs");
	refractShader = Shader("ShaderRefraction.vs", "ShaderRefraction.fs");
	GMTestShader = Shader("ShaderGeometryTest.vs", "ShaderGeometryTest.fs", "ShaderGeometryTest.gs");
	normalShader = Shader("ShaderNormal.vs", "ShaderNormal.fs", "ShaderNormal.gs");
	InstanceShader = Shader("ShaderInstance.vs", "ShaderInstance.fs");
	lightInstShader = Shader("ShaderLightingInstance.vs", "ShaderLightingInstance.fs", "ShaderLightingInstance.gs");

	/* 加载贴图 */
	// 翻转y轴，使图片和opengl坐标一致  但是如果assimp 导入模型时设置了aiProcess_FlipUVs，就不能重复设置了
	stbi_set_flip_vertically_on_load(true);

	// 加载贴图
	GLuint t_metal = 0;
	GLuint t_marble = 0;
	GLuint t_dummy = 0;
	GLuint t_window = 0;

	LoadTexture("Resource/Texture/metal.png", t_metal, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/marble.jpg", t_marble, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/dummy.png", t_dummy, GL_REPEAT, GL_REPEAT);  //自己做的占位贴图，占一个sampler位置，否则会被其他mesh的高光贴图替代
	LoadTexture("Resource/Texture/window.png", t_window, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

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
	// 用于不需要texture的mesh
	const vector<Texture> dummyTexture =
	{
		{t_dummy, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

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

	plane = Mesh(g_planeVertices, g_planeIndices, planeTexture);
	plane.SetScale(vec3(100.0f, 0.0f, 100.0f));
	cubeReflect = Mesh(g_cubeVertices, g_cubeIndices, skyboxTexture);
	cubeMarble = Mesh(g_cubeVertices, g_cubeIndices, cubeTexture);
	square = Mesh(g_squareVertices, g_squareIndices, windowTexture);
	skybox = Mesh(g_skyboxVertices, g_skyboxIndices, skyboxTexture);
	screen = Mesh(g_screenVertices, g_screenIndices, dummyTexture);
	mirror = Mesh(g_mirrorVertices, g_mirrorIndices, dummyTexture);
	particle = Mesh(g_particleVertices, g_particleIndices, dummyTexture);
	GMTest = Mesh(g_GMTestVertices, g_GMTestIndices, dummyTexture);
	InstanceTest = Mesh(g_InstanceTestV, g_InstanceTestI, g_InstanceTestP, dummyTexture, instanceArray);
	

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
	planet.SetScale(vec3(4.0f, 4.0f, 4.0f));

	CreateAsteroid();
	rock = Model("Resource/Model/rock/rock.obj", instMat4);

}

void Scene::DrawScene()
{
	if (bBlending)
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

	// 绘制地板
	//plane.DrawMesh(lightShader);

	// 绘制立方体
	cubeReflect.SetTranslate(vec3(1.0f, 1.5f, 1.0f));
	cubeReflect.DrawMesh(reflectShader, GL_TRIANGLES);
	cubeReflect.DrawMesh(normalShader, GL_TRIANGLES);
	cubeReflect.SetTranslate(vec3(0.0f, 1.5f, -1.0f));
	cubeReflect.DrawMesh(refractShader, GL_TRIANGLES);
	cubeReflect.DrawMesh(normalShader, GL_TRIANGLES);

	cubeMarble.SetTranslate(vec3(3.0f, 1.5f, 0.0f));
	cubeMarble.DrawMesh(lightShader, GL_TRIANGLES);
	cubeMarble.DrawMesh(normalShader, GL_TRIANGLES);

	glDisable(GL_BLEND);
	// 绘制人物
	nanosuit.SetScale(vec3(0.1f));
	nanosuit.SetTranslate(vec3(1.0f, 1.0f, 0.0f));
	nanosuit.DrawModel(lightShader);
	nanosuit.DrawModel(normalShader);
	nanosuit.SetTranslate(vec3(0.0f, 1.0f, -3.0f));
	nanosuit.DrawModel(reflectShader);
	nanosuit.SetTranslate(vec3(3.0f, 1.0f, -3.0f));
	nanosuit.DrawModel(refractShader);
	glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);

	if (bSkyBox)
	{
		// 绘制天空盒
		skybox.DrawMesh(cubemapShader, GL_TRIANGLES);
	}

	planet.DrawModel(lightShader);

	rock.DrawModel(lightInstShader, true);

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
		square.DrawMesh(lightShader, GL_TRIANGLES);
	}

	/* 一些功能测试 */
	glEnable(GL_PROGRAM_POINT_SIZE);
	//绘制的图元是GL_POINTS。对应的是裁剪空间的归一化坐标（实际是在顶点着色器设定）
	glPointSize(pointSize);
	particle.DrawMesh(screenShader, GL_POINTS);
	glDisable(GL_PROGRAM_POINT_SIZE);

	// Geometry Shader Test
	if (bGMTest)
	{
		GMTest.DrawMesh(GMTestShader, GL_POINTS);
	}
	if (bInstanceTest)
	{
		InstanceTest.UniversalDrawMesh(InstanceShader, GL_TRIANGLES);
	}
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
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
	cubeReflect.DeleteMesh();
	cubeMarble.DeleteMesh();
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
	float radius = 50.0;
	float offset = 10.0f;
	for (unsigned int i = 0; i < ROCK_NUM; i++)
	{
		mat4 model;
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