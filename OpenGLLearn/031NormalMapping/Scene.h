﻿#pragma once

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
	Shader normalShader;
	Shader lightInstShader;
	Shader depthmapShader;
	Shader depthmapDisplayShader;
	Shader depthCubemapShader;

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

	vector<vec3> squarePositions;
	vector<mat4> instMat4;

	vector<Texture> m_brickTexture;

	Camera* myCam;

	void CreateScene(Camera* myCam);
	void DrawScene(bool bDepthmap = false, bool bDepthCubemap = false);
	bool LoadTexture(const string&& filePath, GLuint& texture, const GLint param_s, const GLint param_t);
	GLuint LoadCubemap(const vector<string>& cubemapFaces);
	void DeleteScene();
	void CreateShader();
	void UpdateNMVertices();

private:
	void CreateAsteroid();
	void CreateNMVertices(vector<VertexNM>& verticesNM);
	void CalcTangent(vector<VertexNM>& vertices, vec3& tangent, vec3& bitangent);
};

void Scene::CreateShader()
{

	// 不能声明全局变量，因为shader要调用glfw的api，所以必须在glfw初始化完成后
	// Shaderグローバル変数禁止 コンストラクタはglfw初期化後に呼び出し必須 (glfwのapiが必要のため)
	lightShader = Shader("ShaderLighting.vs", "ShaderLighting.fs", "ShaderLighting.gs");
	screenShader = Shader("ShaderPostProcess.vs", "ShaderPostProcess.fs");
	cubemapShader = Shader("ShaderCubemap.vs", "ShaderCubemap.fs");
	reflectShader = Shader("ShaderReflection.vs", "ShaderReflection.fs");
	refractShader = Shader("ShaderRefraction.vs", "ShaderRefraction.fs");
	normalShader = Shader("ShaderNormal.vs", "ShaderNormal.fs", "ShaderNormal.gs");
	lightInstShader = Shader("ShaderLightingInstance.vs", "ShaderLightingInstance.fs");
	depthmapShader = Shader("ShaderDepthMap.vs", "ShaderDepthMap.fs");
	depthmapDisplayShader = Shader("ShaderDepthmapDisplay.vs", "ShaderDepthmapDisplay.fs");
	depthCubemapShader = Shader("ShaderDepthCubemap.vs", "ShaderDepthCubemap.fs", "ShaderDepthCubemap.gs");
}

void Scene::CreateScene(Camera* myCam)
{
	this->myCam = myCam;
	// 翻转y轴，使图片坐标和opengl坐标一致
	// Y軸を反転して画像座標とOpenGL座標を一致させる
	// ※AssimpのaiProcess_FlipUVsフラグ使用時は二重設定禁止
	stbi_set_flip_vertically_on_load(true);

	GLuint t_metal = 0;
	GLuint t_marble = 0;
	GLuint t_dummy = 0;
	GLuint t_window = 0;
	GLuint t_wood = 0;
	GLuint t_white = 0;
	GLuint t_brick = 0;
	GLuint t_brick_normal = 0;

	LoadTexture("Resource/Texture/metal.png", t_metal, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/marble.jpg", t_marble, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/dummy.png", t_dummy, GL_REPEAT, GL_REPEAT);  //自己做的占位贴图，占一个sampler位置，否则会被其他mesh的高光贴图替代
	LoadTexture("Resource/Texture/window.png", t_window, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	LoadTexture("Resource/Texture/wood.png", t_wood, GL_REPEAT, GL_REPEAT);
	LoadTexture("Resource/Texture/AllWhite.png", t_white, GL_REPEAT, GL_REPEAT);

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
	
	const vector<Texture> dummyTexture =
	{
		{t_dummy, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

	const vector<Texture> lampTexture =
	{
		{t_white, "texture_diffuse"},
		{t_dummy, "texture_specular"}
	};

	const vector<Texture> brickTexture =
	{
		{t_brick, "texture_diffuse"},
		{t_dummy, "texture_specular"},
		{t_brick_normal, "texture_normal"}
	};

	// 深拷贝
	m_brickTexture = brickTexture;

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
	screen = Mesh(g_screenVertices, g_screenIndices, dummyTexture);
	mirror = Mesh(g_mirrorVertices, g_mirrorIndices, dummyTexture);
	particle = Mesh(g_particleVertices, g_particleIndices, dummyTexture);
	lamp = Mesh(g_cubeVertices, g_cubeIndices, lampTexture);
	lamp.SetScale(vec3(1.0f));

	if (bEnableNormalMap)
	{
		CreateNMVertices(g_planeVerticesNM);
		PosYSquare = Mesh(g_planeVerticesNM, brickTexture);
	}
	else
	{
		PosYSquare = Mesh(g_planeVertices, g_planeIndices, brickTexture);
	}
	
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

	planet = Model("Resource/Model/planet/planet.obj");
	planet.SetTranslate(vec3(40.0f, 40.0f, 40.0f));
	planet.SetScale(vec3(20.0f, 20.0f, 20.0f));

	CreateAsteroid();
	rock = Model("Resource/Model/rock/rock.obj", instMat4);

}

void Scene::DrawScene(bool bDepthmap, bool bDepthCubemap)
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

	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	// 床の描画
	// 对地板不绘制深度图
	// 床には深度マッピングをしない
	plane.DrawMesh(lightShader, GL_TRIANGLES); 

	// 立方体を描画
	cubeCubemap.SetTranslate(vec3(1.0f, 1.5f, 1.0f));
	if (bDepthmap)
		cubeCubemap.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		cubeCubemap.DrawMesh(depthCubemapShader, GL_TRIANGLES);
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
	else
	{
		cube.DrawMesh(lightShader, GL_TRIANGLES);
		cube.DrawMesh(normalShader, GL_TRIANGLES);
	}

	glDisable(GL_BLEND);

	// キャラクター描画
	nanosuit.SetScale(vec3(0.1f));
	nanosuit.SetTranslate(vec3(1.0f, 1.0f, 0.0f));
	if (bDepthmap)
		nanosuit.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		nanosuit.DrawModel(depthCubemapShader);
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
	else
	{
		nanosuit.DrawModel(reflectShader);
	}
	nanosuit.SetTranslate(vec3(3.0f, 1.0f, -3.0f));
	if (bDepthmap)
		nanosuit.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		nanosuit.DrawModel(depthCubemapShader);
	else
	{
		nanosuit.DrawModel(refractShader);
	}
	
	glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);
	if (bSkyBox)
	{
		// skyboxを描画
		skybox.DrawMesh(cubemapShader, GL_TRIANGLES);
	}
	if (bFaceCulling)
		glEnable(GL_CULL_FACE);

	// 虽然每帧固定旋转一定角度很方便，但是会导致旋转速度受帧数影响，一般不用这种方法
	// フレームごとに固定角度で回転させる方法は便利だが、回転速度がフレームレートに依存するため通常非推奨
	// planet.AddRotate(ROTATE_SPEED_PLANET * deltaTime, vec3(0.0f, 1.0f, 0.0f)); 
	
	// 一般用每秒固定旋转一定角度的方式，这样虽然帧率低的时候会卡，但是不会影响游戏逻辑
	// 一般的には秒間固定角度回転方式を採用。フレームレート低下が発生するが、ゲームロジックへの影響はない
	planet.AddRotate(ROTATE_SPEED_PLANET * deltaTime, vec3(0.0f, 1.0f, 0.0f));
	if (bDepthmap)
		planet.DrawModel(depthmapShader);
	else if (bDepthCubemap)
		planet.DrawModel(depthCubemapShader);
	else
	{
		planet.DrawModel(lightShader);
	}

	if (bDepthmap)
		rock.DrawModel(depthmapShader, true);
	else if (bDepthCubemap)
		rock.DrawModel(depthCubemapShader, true);
	else
		rock.DrawModel(lightInstShader, true);

	for (int i = 0; i < 4; i++)
	{
		lamp.SetTranslate(lampPos[i]);
		if (bDepthmap)
			lamp.DrawMesh(depthmapShader, GL_TRIANGLES);
		else if (bDepthCubemap)
			lamp.DrawMesh(depthCubemapShader, GL_TRIANGLES);
		else
			lamp.DrawMesh(lightShader, GL_TRIANGLES);
	}

	lamp.SetTranslate(lampWithShadowPos);
	if (bDepthmap)
		lamp.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		lamp.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else
		lamp.DrawMesh(lightShader, GL_TRIANGLES);		

	PosYSquare.SetTranslate(vec3(6.0f, 5.0f, 6.0f));
	if (bDepthmap)
		PosYSquare.DrawMesh(depthmapShader, GL_TRIANGLES);
	else if (bDepthCubemap)
		PosYSquare.DrawMesh(depthCubemapShader, GL_TRIANGLES);
	else
	{
		PosYSquare.DrawMesh(lightShader, GL_TRIANGLES);
		PosYSquare.DrawMesh(normalShader, GL_TRIANGLES);
	}

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
		if (bDepthmap)
			square.DrawMesh(depthmapShader, GL_TRIANGLES);
		else if (bDepthCubemap)
			square.DrawMesh(depthCubemapShader, GL_TRIANGLES);
		else
			square.DrawMesh(lightShader, GL_TRIANGLES);
	}


	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(pointSize);
	particle.DrawMesh(screenShader, GL_POINTS);
	glDisable(GL_PROGRAM_POINT_SIZE);
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

	if (bDebug)
	{
		for (uint i = 0; i < height; i++)
		{
			cout << endl;
			for (uint j = 0; j < width; j++)
			{
				unsigned char ch = data[i * width + j];
				printf("%4d ", ch);
			}
		}
		cout << endl;
	}


	GLenum informat = 0;
	GLenum format = 0;
	if (channel == 1)
	{
		informat = GL_RED;
		format = GL_RED;
	}
	else if (channel == 3)
	{
		// 参照 Referrence/Gamma Correction.png
		// 硬盘中的图片是经过gamma correction 1/2.2的，再经过显示器的crt gamma2.2处理后正好恢复正常颜色
		// 这种情况再在渲染程序中gamma校正，就相当于2次gamma校正，所以画面会发白
		// 因此需要指定GL_SRGB，将图片转为线性空间（去除图片本身的gamma correction 1/2.2）
		// 说白了，显示出来是正常的图片，实际上是经过gamma校正的发白图片，如果我们想自己靠渲染实现gamma校正的话，就不需要图片本身的gamma校正了
		// 
		// ディスク上の画像はガンマ補正1/2.2が適用済み。ディスプレイのCRTガンマ2.2処理で正常色復元  
		// レンダリングで追加ガンマ補正すると二重補正となり白飛び発生  
		// 対策：GL_SRGB指定で画像をリニア空間変換（元の1/2.2補正を除去）  
		// 結論：正常表示の画像は既に補正済みのため、レンダリング側で新規補正する場合はテクスチャの補正除去が必要 
		informat = GL_SRGB; 

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
		/* テクスチャデータをVRAMに転送する */
		glTexImage2D(GL_TEXTURE_2D, 0, informat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

		GLenum informat = 0;
		GLenum format = 0;
		if (channel == 1)
		{
			informat = GL_RED;
			format = GL_RED;
		}
		else if (channel == 3)
		{
			informat = GL_SRGB;
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
		    /* テクスチャデータをVRAMに転送する */
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, informat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
						  // 乱数シード初期化
	float radius = 150.0;
	float offset = 30.0f;
	for (unsigned int i = 0; i < ROCK_NUM; i++)
	{
		mat4 model = mat4(1.0f);
		// 1. 位移：分布在半径为 'radius' 的圆形上，偏移的范围是 [-offset, offset]
		// 1. 平行移動：半径'radius'の円周上に分布、オフセット範囲[-offset, offset]
		float angle = (float)i / (float)ROCK_NUM * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // 让行星带的高度比x和z的宽度要小
									   // ベルトの高さをx/z軸より低く抑える
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = translate(model, vec3(x, y, z) + vec3(40.0f, 45.0f, 40.0f));

		// 2. 缩放：在 0.05 和 0.25f 之间缩放
		// 2. スケーリング：0.05 ～ 0.25f の範囲でランダム
		float _scale = (rand() % 20) / 100.0f + 0.05;
		model = scale(model, vec3(_scale));

		// 3. 旋转：绕着一个（半）随机选择的旋转轴向量进行随机的旋转
		// 3. 回転：ランダムな軸ベクトルで回転
		float rotAngle = (rand() % 360);
		model = rotate(model, rotAngle, vec3(0.4f, 0.6f, 0.8f));

		// 4. 添加到矩阵的数组中
		// 4. インスタンシング用行列配列に追加
		instMat4.push_back(model);
	}
}

// 参照 Referrence/tangent space.png
void Scene::CalcTangent(vector<VertexNM>& vertices, vec3& tangent, vec3& bitangent)
{
	if (vertices.size() != 3)
	{
		cout << "CalcTangent Error: Vertices size is not 3!" << endl;
		return;
	}

	// 取出顶点和UV信息
	// 頂点とUV情報を抽出
	vec3 pos1 = vertices[0].position;
	vec3 pos2 = vertices[1].position;
	vec3 pos3 = vertices[2].position;
	vec2 uv1 = vertices[0].texCoord;
	vec2 uv2 = vertices[1].texCoord;
	vec2 uv3 = vertices[2].texCoord;

	// 1 edge  deltaUV
	vec3 edge1 = pos2 - pos1;
	vec3 edge2 = pos3 - pos1;
	vec2 deltaUV1 = uv2 - uv1;
	vec2 deltaUV2 = uv3 - uv1;

	// 2 Tangent  bitangent
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
		vector<VertexNM> temp_vertices;
		vec3 tangent, bitangent;
		temp_vertices.push_back(verticesNM[i]);
		temp_vertices.push_back(verticesNM[i + 1]);
		temp_vertices.push_back(verticesNM[i + 2]);
		CalcTangent(temp_vertices, tangent, bitangent);

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