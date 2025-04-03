#pragma once

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "common.h"
#include "assimp/types.h"



struct Vertex
{
	vec3 position; //位置
	vec3 normal;   //法線
	vec2 texCoord; //UV座標
};

struct VertexNM
{
	vec3 position; //位置
	vec3 normal;   //法線
	vec2 texCoord; //UV座標
	vec3 tangent;  // 接線 
	vec3 bitangent; //従接線
};


struct Texture
{
	unsigned int id; 
	string type;   
	aiString path;
};
#pragma pack()

class Mesh
{
public:
	Mesh();
	Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, 
		 const vector<Texture>& textures = {}, const vector<mat4>& instMat4 = {});
	Mesh(const vector<VertexNM>& verticesNM, const vector<Texture>& textures = {});
	~Mesh();

	void DrawMesh(const Shader& shader, GLuint element, bool bInst = false);
	void DeleteMesh();

	void SetScale(vec3 scale);
	void SetTranslate(vec3 scale);
	void SetRotate(float angle, vec3 axis);
	void SetModel(mat4 model);
	void SetTextures(const vector<Texture>& textures);
	void AddTextures(const vector<Texture>& textures);
	void SetInstMat4();
	void AddRotate(float angle, vec3 axis);
	void CalcTangent(const vector<Vertex>& vertices, vec3& tangent, vec3& bitangent);

protected:  //サブクラスしかアクセスできない
	vec3 m_scale;
	vec3 m_translate;
	float m_rotateAngle;
	vec3 m_rotateAxis;
	mat4 m_model;

private:
	void SetupMesh(bool bInst = false);
	void SetupMeshNM();

	vector<Vertex> vertices;
	vector<VertexNM> verticesNM;
	vector<GLuint> indices;
	vector<uint> parse;
	vector<Texture> textures;
	vector<vec2> instanceArray;
	vector<mat4> instMat4;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint VBO_Instances;
	GLuint VBO_InstMat4;
};

Mesh::Mesh()
{
	VAO = 0;
	VBO = 0;
	EBO = 0;
	VBO_Instances = 0;

	m_scale = vec3(1.0f);
	m_translate = vec3(0.0f);
	m_rotateAngle = 0.0f;
	m_rotateAxis = vec3(1.0, 1.0, 1.0);
	m_model = mat4(1.0f);
}

Mesh::Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, const vector<Texture>& textures, const vector<mat4>& instMat4)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->instMat4 = instMat4;

	VAO = 0;
	VBO = 0;
	EBO = 0;
	VBO_Instances = 0;

	m_scale = vec3(1.0f);
	m_translate = vec3(0.0f);
	m_rotateAngle = 0.0f;
	m_rotateAxis = vec3(1.0, 1.0, 1.0);
	m_model = mat4(1.0f);

	if (instMat4.empty())
		SetupMesh();
	else
		SetupMesh(true);
}

Mesh::Mesh(const vector<VertexNM>& verticesNM, const vector<Texture>& textures)
{
	this->verticesNM = verticesNM;
	this->textures = textures;

	VAO = 0;
	VBO = 0;
	EBO = 0;
	VBO_Instances = 0;

	m_scale = vec3(1.0f);
	m_translate = vec3(0.0f);
	m_rotateAngle = 0.0f;
	m_rotateAxis = vec3(1.0, 1.0, 1.0);
	m_model = mat4(1.0f);
	
	SetupMeshNM();
}

Mesh::~Mesh()
{
}

void Mesh::SetupMesh(bool bInst)
{
	// 参照 Referrence/opengl vertex management.png
	// 用VAO来管理 shader的顶点属性
	// VAOを使用してシェーダーの頂点属性を管理
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); 

	// 存储顶点数据到VBO
	// 頂点データをVBOに格納	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// 存储下标数据到EBO
	// インデックスデータをEBOに格納
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	glVertexAttribPointer(0, sizeof(((Vertex*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);
	// 3忘记加sizeof(GL_FLOAT)了，排查了半天。。。以后0也写成0 * sizeof(GL_FLOAT)的形式吧。。以免误导别的代码
	// 3のsizeof(GL_FLOAT)を書き忘れて半日デバッグした…今後は0も「0 * sizeof(GL_FLOAT)」形式で書こう…他コードの誤解防止のため 
	glVertexAttribPointer(1, sizeof(((Vertex*)0)->normal) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, sizeof(((Vertex*)0)->texCoord) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
	glEnableVertexAttribArray(2);

	if (bInst)
		SetInstMat4();

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// 设置支持法线贴图的mesh，暂不同时支持instance化
// 法線マップ対応メッシュのセットアップ（暫定的にインスタンシング非対応）
void Mesh::SetupMeshNM()
{
	// 参照 Referrence/opengl vertex management.png
	// 用VAO来管理 shader的顶点属性
	// VAOを使用してシェーダーの頂点属性を管理
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// 存储顶点数据到VBO
	// 頂点データをVBOに格納	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexNM) * verticesNM.size(), &verticesNM[0], GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	glVertexAttribPointer(0, sizeof(((VertexNM*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(VertexNM), (void*)(offsetof(VertexNM, position)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, sizeof(((VertexNM*)0)->normal) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(VertexNM), (void*)(offsetof(VertexNM, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, sizeof(((VertexNM*)0)->texCoord) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(VertexNM), (void*)(offsetof(VertexNM, texCoord)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, sizeof(((VertexNM*)0)->tangent) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(VertexNM), (void*)(offsetof(VertexNM, tangent)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, sizeof(((VertexNM*)0)->bitangent) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(VertexNM), (void*)(offsetof(VertexNM, bitangent)));
	glEnableVertexAttribArray(4);


	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(const Shader& shader, GLuint element, bool bInst)
{
	// 设置纹理单元 任何uniform设置操作一定要放到《对应的shader》有效之后！  --》不同的shader切换运行，另一个shader会关掉，写的数据会丢失数据
	//也就是说启动了shader1之后又启动了shader2，之前在shader1设置的就无效了！这种情况只能放到渲染循环里，不能放循环外面
	// テクスチャユニットの設定：ユニフォーム変数の操作は必ず《対応するシェーダー》有効中に行う！  
	// → 別のシェーダーに切り替えると設定値が失われる  
	// 例: shader1起動後にshader2を起動 → shader1の設定は無効化  
	// 解決策: レンダリングループ内で対応するシェーダー有効中で設定（ループ外では不可）
	glBindVertexArray(VAO); 
	shader.Use();
	uint diffuseN = 0;
	uint specularN = 0;
	uint reflectionN = 0;
	uint cubemapN = 0;
	uint normalN = 0;
	uint dispN = 0;
	string type;
	bool bNM = false;
	bool bDisp = false;

	for (uint i = 0; i < textures.size(); i++)
	{
		type = textures[i].type;
		shader.SetBool("bNormalMap", false);
		shader.SetBool("bParallaxMap", false);

		if (type == "texture_diffuse")
		{
			//cout << "texture_diffuse" << endl;
			diffuseN++;
			shader.SetInt("material." + type + to_string(diffuseN), i);   // 不清楚这里一次draw有多个贴图要怎么搞，这里代码姑且保留
		}
		else if (type == "texture_specular")
		{
			//cout << "texture_specular" << endl;
			specularN++;
			shader.SetInt("material." + type + to_string(specularN), i);
		}
		else if (type == "texture_reflection")
		{
			//cout << "texture_reflection" << endl;
			reflectionN++;
			shader.SetInt("material." + type + to_string(reflectionN), i);
		}
		else if (type == "texture_cubemap")
		{
			//cout << "texture_cubemap" << endl;
			cubemapN++;
			shader.SetInt(type + to_string(cubemapN), i);
		}
		else if (type == "texture_normal" && bEnableNormalMap)
		{
			//cout << "texture_normal" << endl;
			normalN++;
			shader.SetInt("material." + type + to_string(normalN), i);
			bNM = true;
		}
		else if (type == "texture_disp" && bEnableNormalMap && bEnableParallaxMap)
		{
			//cout << "texture_disp" << endl;
			dispN++;
			shader.SetInt("material." + type + to_string(dispN), i);
			bDisp = true;
		}

		//cout << i << endl << endl;
		glActiveTexture(GL_TEXTURE0 + i);
		if (type == "texture_cubemap")
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, textures[i].id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}	
	}
	//cout << "***********************************" << endl;

	if (bNM)
		shader.SetInt("bNormalMap", true);
	if (bDisp)
		shader.SetInt("bParallaxMap", true);

	if (!bInst)
	{
		mat4 model = mat4(1.0f);
		model = translate(model, m_translate);
		model = scale(model, m_scale);
		model = rotate(model, m_rotateAngle, m_rotateAxis);
		shader.SetMat4("uni_model", model);
		if (!bNM)
			glDrawElements(element, indices.size(), GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(GL_TRIANGLES, 0, verticesNM.size());
	}
	else if (!bNM) // インスタンシングと法線マップの併用は暫定非対応
	{
		// uniform変数不要・モデル行列はインスタンシング配列属性で渡す
		glDrawElementsInstanced(element, indices.size(), GL_UNSIGNED_INT, 0, ROCK_NUM);
	}

	// unbind
	diffuseN = 0;
	specularN = 0;
	reflectionN = 0;
	cubemapN = 0;
	normalN = 0;
	dispN = 0;

	for (uint i = 0; i < textures.size(); i++)
	{
		type = textures[i].type;
		if (type == "texture_diffuse")
		{
			//cout << "texture_diffuse" << endl;
			diffuseN++;
			shader.SetInt("material." + type + to_string(diffuseN), i);   // 不清楚这里一次draw有多个贴图要怎么搞，这里代码姑且保留
		}
		else if (type == "texture_specular")
		{
			//cout << "texture_specular" << endl;
			specularN++;
			shader.SetInt("material." + type + to_string(specularN), i);
		}
		else if (type == "texture_reflection")
		{
			//cout << "texture_reflection" << endl;
			reflectionN++;
			shader.SetInt("material." + type + to_string(reflectionN), i);
		}
		else if (type == "texture_cubemap")
		{
			//cout << "texture_cubemap" << endl;
			cubemapN++;
			shader.SetInt(type + to_string(cubemapN), i);
		}
		else if (type == "texture_normal" && bEnableNormalMap)
		{
			//cout << "texture_normal" << endl;
			normalN++;
			shader.SetInt(type + to_string(normalN), i);
		}
		else if (type == "texture_disp" && bEnableNormalMap && bEnableParallaxMap)
		{
			//cout << "texture_disp" << endl;
			dispN++;
			shader.SetInt("material." + type + to_string(dispN), i);
		}

		//cout << i << endl << endl;
		glActiveTexture(GL_TEXTURE0 + i);
		if (type == "texture_cubemap")
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	glBindVertexArray(0);
}

void Mesh::DeleteMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO); 
	glDeleteBuffers(1, &VBO_Instances);
	for (int i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i].id);
	}
}

void Mesh::SetScale(vec3 scale)
{
	m_scale = scale;
}

void Mesh::SetTranslate(vec3 translate)
{
	m_translate = translate;
}

void Mesh::SetRotate(float angle, vec3 axis)
{
	m_rotateAngle = angle;
	m_rotateAxis = axis;
}

void Mesh::AddRotate(float angle, vec3 axis)
{
	m_rotateAngle += angle;
	m_rotateAxis = axis;
}

void Mesh::SetModel(mat4 model)
{
	m_model = model;
}

void Mesh::SetTextures(const vector<Texture>& textures)
{
	this->textures = textures;
}

void Mesh::AddTextures(const vector<Texture>& textures)
{
	this->textures.insert(this->textures.end(), textures.begin(), textures.end());
}

void Mesh::SetInstMat4()
{
	/**************************** 实例化数组 ****************************/
	// インスタンシング配列

	// 存储实例化数组到显存VBO
	// インスタンシング配列をVRAMのVBOに格納
	glGenBuffers(1, &VBO_InstMat4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_InstMat4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * instMat4.size(), &instMat4[0], GL_STATIC_DRAW);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vec4) * 4, (void*)(sizeof(vec4) * 0));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vec4) * 4, (void*)(sizeof(vec4) * 1));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(vec4) * 4, (void*)(sizeof(vec4) * 2));
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vec4) * 4, (void*)(sizeof(vec4) * 3));
	glEnableVertexAttribArray(6);
	glVertexAttribDivisor(6, 1);
}

// 参照 Referrence/tangent space.png
void Mesh::CalcTangent(const vector<Vertex>& vertices, vec3& tangent, vec3& bitangent)
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

	//  edge  deltaUV
	vec3 edge1 = pos2 - pos1;
	vec3 edge2 = pos3 - pos1;
	vec2 deltaUV1 = uv2 - uv1;
	vec2 deltaUV2 = uv3 - uv1;

	// Tangent bitangent
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