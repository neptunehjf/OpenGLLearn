#pragma once

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "glfw/glfw3.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


//顶点数据
#pragma pack(1)
struct Vertex
{
	glm::vec3 position; //位置
	glm::vec3 normal;   //法线
	glm::vec2 texCoord; //纹理坐s标
};
#pragma pack()

//贴图数据
#pragma pack(1)
struct Texture
{
	unsigned int id; //贴图id
	std::string type; //贴图类型，比如 漫反射贴图 还是 高光贴图
};
#pragma pack()

class Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures);
	~Mesh();

	void SetupMesh();
	void DrawMesh(Shader shader, Shader shader_lamp, float posValue);
	void DeleteMesh() const;

private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint lampVAO;
};

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures)
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

	//光源模型，一个白色的发光体
    // 专门为光源定义了一个VAO，方便后续操作
	glGenVertexArrays(1, &lampVAO);
	glBindVertexArray(lampVAO); // VBO glVertexAttribPointer 操作向VAO上下文写

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glVertexAttribPointer(0, sizeof(((Vertex*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);

	// 解绑
	glBindVertexArray(0);// 关闭VAO上下文

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//当目标是GL_ELEMENT_ARRAY_BUFFER的时候，VAO会储存glBindBuffer的函数调用。这也意味着它也会储存解绑调用，所以确保你没有在解绑VAO之前解绑索引数组缓冲，否则它就没有这个EBO配置了
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(Shader shader, Shader shader_lamp, float posValue)
{
	// 设置纹理单元 任何uniform设置操作一定要放到《对应的shader》启动之后！  --》不同的shader切换运行，另一个shader会关掉，写的数据会丢失数据
    // 也就是说启动了shader之后又启动了shader_lamp，之前在shader设置的就无效了！这种情况只能放到渲染循环里，不能放循环外面
	glBindVertexArray(VAO); // draw操作从VAO上下文读顶点数据    可代替VBO EBO attrpoint的绑定操作，方便管理
	shader.Use();
	GLuint diffuseN = 0;
	GLuint specularN = 0;
	std::string type;
	for (int i = 0; i < textures.size(); i++)
	{
		type = textures[i].type;
		if (type == "texture_diffuse")
		{
			diffuseN++;
			shader.SetInt("material." + type + std::to_string(diffuseN), i);
		}
		else if (type == "texture_specular")
		{
			shader.SetInt("material." + type + std::to_string(specularN), i);
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);  //片段着色器会根据对应的纹理单元读取texture_diffuse的贴图数据
	}

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// 解绑
	glBindVertexArray(0);

	glBindVertexArray(lampVAO);
	shader_lamp.Use();
	for (int i = 0; i < 4; i++)
	{
		std::stringstream ss;
		ss << "pointLight[" << i << "].";
		std::string prefix = ss.str();

		glm::vec3 lightPos = glm::vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));

		glm::mat4 model = glm::mat4(1.0f);  // 初始化为单位矩阵，清空
		model = scale(model, glm::vec3(0.5f));
		model = translate(model, lightPos);
		model = rotate(model, glm::radians(45.0f + i * 10), glm::vec3(1.0f, 1.0f, 0.0f));

		shader_lamp.SetMat4("uni_model", model);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	// 解绑
	glBindVertexArray(0);
}

void Mesh::DeleteMesh() const
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO); 
	glDeleteBuffers(1, &lampVAO);
	glDeleteTextures(1, &textures[0].id);
	glDeleteTextures(1, &textures[1].id);
}
