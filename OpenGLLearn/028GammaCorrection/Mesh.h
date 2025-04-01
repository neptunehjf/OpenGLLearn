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


struct Texture
{
	unsigned int id; 
	string type;   
	aiString path;
};

class Mesh
{
public:
	Mesh();
	Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, 
		 const vector<Texture>& textures, const vector<mat4>& instMat4 = {});
	Mesh(const vector<float>& vertices, const vector<uint>& indices, const vector<uint>& parse, 
		 const vector<Texture>& textures, const vector<vec2>& instanceArray);
	~Mesh();

	void DrawMesh(const Shader& shader, GLuint element, bool bInst = false);
	void UniversalDrawMesh(const Shader& shader, GLuint element);
	void DeleteMesh();

	void SetScale(vec3 scale);
	void SetTranslate(vec3 scale);
	void SetRotate(float angle, vec3 axis);
	void SetModel(mat4 model);
	void SetTextures(const vector<Texture>& textures);
	void AddTextures(const vector<Texture>& textures);
	void SetInstMat4();
	void AddRotate(float angle, vec3 axis);

protected:  //サブクラスしかアクセスできない
	vec3 m_scale;
	vec3 m_translate;
	float m_rotateAngle;
	vec3 m_rotateAxis;
	mat4 m_model;

private:
	void SetupMesh(bool bInst = false);
	void UniversalSetupMesh();

	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<float> u_vertices;
	vector<uint> u_indices;
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

Mesh::Mesh(const vector<float>& vertices, const vector<uint>& indices, const vector<uint>& parse,
		   const vector<Texture>& textures, const vector<vec2>& instanceArray)
{
	this->u_vertices = vertices;
	this->u_indices = indices;
	this->parse = parse;
	this->textures = textures;
	this->instanceArray = instanceArray;

	VAO = 0;
	VBO = 0;
	EBO = 0;
	VBO_Instances = 0;

	m_scale = vec3(1.0f);
	m_translate = vec3(0.0f);
	m_rotateAngle = 0.0f;
	m_rotateAxis = vec3(1.0, 1.0, 1.0);
	m_model = mat4(1.0f);

	UniversalSetupMesh();
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

void Mesh::UniversalSetupMesh()
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * u_vertices.size(), &u_vertices[0], GL_STATIC_DRAW);

	// 存储下标数据到EBO
	// インデックスデータをEBOに格納
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * u_indices.size(), &u_indices[0], GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	uint stride = 0;
	for (uint i = 0; i < parse.size(); i++)
	{
		stride += parse[i];
	}
	uint offset = 0;

	for (uint i = 0; i < parse.size(); i++)
	{
		glVertexAttribPointer(i, parse[i], GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * offset));
		glEnableVertexAttribArray(i);
		offset += parse[i];
	}

	/**************************** 实例化数组 ****************************/
	// インスタンシング配列

	// 存储实例化数组到显存VBO
	// インスタンシング配列をVRAMのVBOに格納
	glGenBuffers(1, &VBO_Instances);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Instances);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * instanceArray.size(), &instanceArray[0], GL_STATIC_DRAW);

	// parse.size()正好对应实例化数组的layout location 
	// parse.size()はインスタンシング配列のlayout locationに対応
	int componetNum = sizeof(instanceArray[0]) / sizeof(GL_FLOAT);
	glVertexAttribPointer(parse.size(), componetNum, GL_FLOAT, GL_FALSE, sizeof(float) * componetNum, (void*)(sizeof(float) * 0));
	glEnableVertexAttribArray(parse.size());
	// 指定location2 每渲染1个实例更新1次instanceArray，第二个参数是0的话等于没调用，就是每渲染一个顶点更新1次实例数组了，会出bug
	// location2の更新頻度指定（第2引数が0だと未定義動作：頂点ごとにインスタンス配列更新→バグ発生）
	glVertexAttribDivisor(2, 1);
	
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Mesh::UniversalDrawMesh(const Shader& shader, GLuint element)
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
	GLuint reflectionN = 0;
	GLuint cubemapN = 0;
	string type;

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

	shader.SetMat4("uni_model", m_model);

	glDrawElements(element, indices.size(), GL_UNSIGNED_INT, 0);
	//glDrawElementsInstanced(element, u_indices.size(), GL_UNSIGNED_INT, 0, 100);

	diffuseN = 0;
	specularN = 0;
	reflectionN = 0;
	cubemapN = 0;
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
	GLuint diffuseN = 0;
	GLuint specularN = 0;
	GLuint reflectionN = 0;
	GLuint cubemapN = 0;
	string type;

	for (uint i = 0; i < textures.size(); i++)
	{
		type = textures[i].type;
		if (type == "texture_diffuse")
		{
			//cout << "texture_diffuse" << endl;
			diffuseN++;
			shader.SetInt("material." + type + to_string(diffuseN), i);
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

	if (!bInst)
	{
		mat4 model = mat4(1.0f);
		model = translate(model, m_translate);
		model = scale(model, m_scale);
		model = rotate(model, m_rotateAngle, m_rotateAxis);
		shader.SetMat4("uni_model", model);
		glDrawElements(element, indices.size(), GL_UNSIGNED_INT, 0);
	}
	else
	{
		// 不需要set uniform ，model作为实例化数组属性传入
		glDrawElementsInstanced(element, indices.size(), GL_UNSIGNED_INT, 0, ROCK_NUM);
	}

	// unbind
	diffuseN = 0;
	specularN = 0;
	reflectionN = 0;
	cubemapN = 0;
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