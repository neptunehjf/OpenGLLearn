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

//顶点数据
#pragma pack(1)
struct Vertex
{
	vec3 position; //位置
	vec3 normal;   //法线
	vec2 texCoord; //纹理坐标
};
#pragma pack()

//贴图数据
#pragma pack(1)
struct Texture
{
	unsigned int id; //贴图id
	string type;     //贴图类型，比如 漫反射贴图 还是 高光贴图
	aiString path;     //贴图路径
};
#pragma pack()

class Mesh
{
public:
	Mesh();
	Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, const vector<Texture>& textures);
	~Mesh();

	void DrawMesh(const Shader& shader);
	void DeleteMesh();

	void SetScale(vec3 scale);
	void SetTranslate(vec3 scale);
	void SetTextures(const vector<Texture>& textures);
	void AddTextures(const vector<Texture>& textures);

protected:  //只允许子类访问
	vec3 m_scale;
	vec3 m_translate;

private:
	void SetupMesh();

	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint lampVAO;
};

Mesh::Mesh()
{
	VAO = 0;
	VBO = 0;
	EBO = 0;
	lampVAO = 0;

	m_scale = vec3(1.0f);
	m_translate = vec3(0.0f);
}

Mesh::Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, const vector<Texture>& textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	VAO = 0;
	VBO = 0;
	EBO = 0;
	lampVAO = 0;

	m_scale = vec3(1.0f);
	m_translate = vec3(1.0f);

	SetupMesh();
}

Mesh::~Mesh()
{
}

void Mesh::SetupMesh()
{
	// 用显存VAO来管理 shader的顶点属性
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // VBO glVertexAttribPointer 操作向VAO上下文写

	// 存储顶点数据到显存VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// 存储下标数据到显存EBO
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// 定义顶点属性的解析方式
	glVertexAttribPointer(0, sizeof(((Vertex*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);
	//3 忘记加sizeof(GL_FLOAT)了，排查了半天。。。以后0也写成0 * sizeof(GL_FLOAT)的形式吧。。以免误导别的代码
	glVertexAttribPointer(1, sizeof(((Vertex*)0)->normal) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, sizeof(((Vertex*)0)->texCoord) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
	glEnableVertexAttribArray(2);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//当目标是GL_ELEMENT_ARRAY_BUFFER的时候，VAO会储存glBindBuffer的函数调用。这也意味着它也会储存解绑调用，所以确保你没有在解绑VAO之前解绑索引数组缓冲，否则它就没有这个EBO配置了
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(const Shader& shader)
{
	// 设置纹理单元 任何uniform设置操作一定要放到《对应的shader》启动之后！  --》不同的shader切换运行，另一个shader会关掉，写的数据会丢失数据
    // 也就是说启动了shader之后又启动了shader_lamp，之前在shader设置的就无效了！这种情况只能放到渲染循环里，不能放循环外面
	glBindVertexArray(VAO); // draw操作从VAO上下文读顶点数据    可代替VBO EBO attrpoint的绑定操作，方便管理
	shader.Use();
	GLuint diffuseN = 0;
	GLuint specularN = 0;
	GLuint reflectionN = 0;
	GLuint cubemapN = 0;
	string type;

	for (int i = 0; i < textures.size(); i++)
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

	mat4 model = mat4(1.0f);  
	model = translate(model, m_translate);
	model = scale(model, m_scale);
	shader.SetMat4("uni_model", model);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// 解绑
	if (type == "texture_cubemap")
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
}

void Mesh::DeleteMesh()
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

void Mesh::SetScale(vec3 scale)
{
	m_scale = scale;
}

void Mesh::SetTranslate(vec3 translate)
{
	m_translate = translate;
}

void Mesh::SetTextures(const vector<Texture>& textures)
{
	this->textures = textures;
}

void Mesh::AddTextures(const vector<Texture>& textures)
{
	this->textures.insert(this->textures.end(), textures.begin(), textures.end());
}