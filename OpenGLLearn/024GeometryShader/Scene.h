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

	vector<vec3> squarePositions;

	Camera* myCam;

	void CreateScene(Camera* myCam);
	void DrawScene();
	bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t);
	GLuint LoadCubemap(const vector<string>& cubemapFaces);
	void DeleteScene();
};

void Scene::CreateScene(Camera* myCam)
{
	this->myCam = myCam;

	// 不能声明全局变量，因为shader要调用glfw的api，所以必须在glfw初始化完成后
	// Shaderグローバル変数禁止 コンストラクタはglfw初期化後に呼び出し必須 (glfwのapiが必要のため)
	lightShader = Shader("ShaderLighting.vs", "ShaderLighting.fs", "ShaderLighting.gs");
	screenShader = Shader("ShaderPostProcess.vs", "ShaderPostProcess.fs");
	cubemapShader = Shader("ShaderCubemap.vs", "ShaderCubemap.fs");
	reflectShader = Shader("ShaderReflection.vs", "ShaderReflection.fs");
	refractShader = Shader("ShaderRefraction.vs", "ShaderRefraction.fs");
	GMTestShader = Shader("ShaderGeometryTest.vs", "ShaderGeometryTest.fs", "ShaderGeometryTest.gs");
	normalShader = Shader("ShaderNormal.vs", "ShaderNormal.fs", "ShaderNormal.gs");

	// 翻转y轴，使图片坐标和opengl坐标一致
	// Y軸を反転して画像座標とOpenGL座標を一致させる
	// ※AssimpのaiProcess_FlipUVsフラグ使用時は二重設定禁止
	stbi_set_flip_vertically_on_load(true);

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

	squarePositions.push_back(glm::vec3(-1.5f, 1.0f, -0.48f));
	squarePositions.push_back(glm::vec3(1.5f, 1.0f, 0.51f));
	squarePositions.push_back(glm::vec3(0.0f, 1.0f, 0.7f));
	squarePositions.push_back(glm::vec3(-0.3f, 1.0f, -2.3f));
	squarePositions.push_back(glm::vec3(0.5f, 1.0f, -0.6f));

	nanosuit = Model("Resource/Model/nanosuit_reflection/nanosuit.obj");
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

	// 各バッファをクリア
	glClearColor(bkgColor.r, bkgColor.g, bkgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 床の描画
	//plane.DrawMesh(lightShader);

	// 立方体を描画
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

	// キャラクター描画
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
	
	// skyboxを描画
	skybox.DrawMesh(cubemapShader, GL_TRIANGLES);


	// 按窗户离摄像机间的距离排序，map默认是升序排序，也就是从近到远
	// 必须放在render loop里，因为摄像机是实时改变的
	// ウィンドウオブジェクトとカメラ間の距離でソート（std::mapのデフォルトは昇順=近→遠）
	// カメラ位置がリアルタイム更新されるため、レンダリングループ内で毎フレーム実行必須
	map<float, vec3> sorted;
	for (int i = 0; i < squarePositions.size(); i++)
	{
		float distance = length(myCam->camPos - squarePositions[i]);
		sorted[distance] = squarePositions[i];
	}
	// 透明物体必须最后绘制，并且透明物体之间要从远到近绘制
	// 透明オブジェクトは最終描画かつ、透明同士は遠→近の逆順描画が必要
	for (map<float, vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
	{
		square.SetTranslate(it->second);
		square.DrawMesh(lightShader, GL_TRIANGLES);
	}
}

bool Scene::LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t)
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

GLuint Scene::LoadCubemap(const vector<string>& cubemapFaces)
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