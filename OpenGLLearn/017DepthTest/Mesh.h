#pragma once

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "namespace.h"
#include "assimp/types.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


struct Vertex
{
	vec3 position; //位置
	vec3 normal;   //法線
	vec2 texCoord; //UV座標
};


struct Texture
{
	unsigned int id; 
	string type;   
	aiString path;
};

class Mesh
{
public:
	Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, const vector<Texture>& textures);
	~Mesh();

	void SetupMesh();
	void DrawMesh(const Shader& shader, const Shader& shader_lamp, float posValue);
	void DeleteMesh() const;

private:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint lampVAO;
};

Mesh::Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, const vector<Texture>& textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	VAO = 0;
	VBO = 0;
	EBO = 0;
	lampVAO = 0;

	SetupMesh();
}

Mesh::~Mesh()
{
}

void Mesh::SetupMesh()
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

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);

	// 表示光源的物体
	// 光源を表すオブジェクト
	glGenVertexArrays(1, &lampVAO);
	glBindVertexArray(lampVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glVertexAttribPointer(0, sizeof(((Vertex*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(const Shader& shader, const Shader& shader_lamp, float posValue)
{
	// 设置纹理单元 任何uniform设置操作一定要放到《对应的shader》有效之后！  --》不同的shader切换运行，另一个shader会关掉，写的数据会丢失数据
	//也就是说启动了shader1之后又启动了shader2，之前在shader1设置的就无效了！这种情况只能放到渲染循环里，不能放循环外面
	// テクスチャユニットの設定：ユニフォーム変数の操作は必ず《対応するシェーダー》有効中に行う！  
	// → 別のシェーダーに切り替えると設定値が失われる  
	// 例: shader1起動後にshader2を起動 → shader1の設定は無効化  
	// 解決策: レンダリングループ内で対応するシェーダー有効中で設定（ループ外では不可）
	glBindVertexArray(VAO); 
	shader.Use();
	GLuint diffuseN = 0;
	GLuint specularN = 0;
	string type;

	cout << textures.size() << endl;
	for (int i = 0; i < textures.size(); i++)
	{
		type = textures[i].type;
		if (type == "texture_diffuse")
		{
			diffuseN++;
			string str1 = "material." + type + to_string(diffuseN);
			cout << str1 << endl;
			shader.SetInt("material." + type + to_string(diffuseN), i);
		}
		else if (type == "texture_specular")
		{
			specularN++;
			string str2 = "material." + type + to_string(specularN);
			cout << str2 << endl;
			shader.SetInt("material." + type + to_string(specularN), i);
		}
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	cout << endl;
	mat4 model = mat4(1.0f);           
	model = scale(model, vec3(0.3f));
	shader.SetMat4("uni_model", model);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


	glBindVertexArray(0);

	//glBindVertexArray(lampVAO);
	//shader_lamp.Use();
	//for (int i = 0; i < 4; i++)
	//{
	//	stringstream ss;
	//	ss << "pointLight[" << i << "].";
	//	string prefix = ss.str();

	//	vec3 lightPos = vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));

	//	mat4 model = mat4(1.0f);  // 初始化为单位矩阵，清空
	//	model = scale(model, vec3(0.5f));
	//	model = translate(model, lightPos);
	//	model = rotate(model, radians(45.0f + i * 10), vec3(1.0f, 1.0f, 0.0f));

	//	shader_lamp.SetMat4("uni_model", model);

	//	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	//}


	//glBindVertexArray(0);
}

void Mesh::DeleteMesh() const
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO); 
	glDeleteBuffers(1, &lampVAO);
	for (int i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i].id);
	}
}
