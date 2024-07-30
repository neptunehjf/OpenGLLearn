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


//��������
#pragma pack(1)
struct Vertex
{
	glm::vec3 position; //λ��
	glm::vec3 normal;   //����
	glm::vec2 texCoord; //������s��
};
#pragma pack()

//��ͼ����
#pragma pack(1)
struct Texture
{
	unsigned int id; //��ͼid
	std::string type; //��ͼ���ͣ����� ��������ͼ ���� �߹���ͼ
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
	// ���Դ�VAO������ shader�Ķ�������
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // VBO glVertexAttribPointer ������VAO������д

	// �洢�������ݵ��Դ�VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// �洢�±����ݵ��Դ�EBO
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// ���嶥�����ԵĽ�����ʽ
	glVertexAttribPointer(0, sizeof(((Vertex*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);
	//3 ���Ǽ�sizeof(GL_FLOAT)�ˣ��Ų��˰��졣�����Ժ�0Ҳд��0 * sizeof(GL_FLOAT)����ʽ�ɡ��������󵼱�Ĵ���
	glVertexAttribPointer(1, sizeof(((Vertex*)0)->normal) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, sizeof(((Vertex*)0)->texCoord) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
	glEnableVertexAttribArray(2);

	// ���
	glBindVertexArray(0);// �ر�VAO������

	//��Դģ�ͣ�һ����ɫ�ķ�����
    // ר��Ϊ��Դ������һ��VAO�������������
	glGenVertexArrays(1, &lampVAO);
	glBindVertexArray(lampVAO); // VBO glVertexAttribPointer ������VAO������д

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glVertexAttribPointer(0, sizeof(((Vertex*)0)->position) / sizeof(GL_FLOAT), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);

	// ���
	glBindVertexArray(0);// �ر�VAO������

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//��Ŀ����GL_ELEMENT_ARRAY_BUFFER��ʱ��VAO�ᴢ��glBindBuffer�ĺ������á���Ҳ��ζ����Ҳ�ᴢ������ã�����ȷ����û���ڽ��VAO֮ǰ����������黺�壬��������û�����EBO������
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(Shader shader, Shader shader_lamp, float posValue)
{
	// ��������Ԫ �κ�uniform���ò���һ��Ҫ�ŵ�����Ӧ��shader������֮��  --����ͬ��shader�л����У���һ��shader��ص���д�����ݻᶪʧ����
    // Ҳ����˵������shader֮����������shader_lamp��֮ǰ��shader���õľ���Ч�ˣ��������ֻ�ܷŵ���Ⱦѭ������ܷ�ѭ������
	glBindVertexArray(VAO); // draw������VAO�����Ķ���������    �ɴ���VBO EBO attrpoint�İ󶨲������������
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
		glBindTexture(GL_TEXTURE_2D, textures[i].id);  //Ƭ����ɫ������ݶ�Ӧ������Ԫ��ȡtexture_diffuse����ͼ����
	}

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// ���
	glBindVertexArray(0);

	glBindVertexArray(lampVAO);
	shader_lamp.Use();
	for (int i = 0; i < 4; i++)
	{
		std::stringstream ss;
		ss << "pointLight[" << i << "].";
		std::string prefix = ss.str();

		glm::vec3 lightPos = glm::vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));

		glm::mat4 model = glm::mat4(1.0f);  // ��ʼ��Ϊ��λ�������
		model = scale(model, glm::vec3(0.5f));
		model = translate(model, lightPos);
		model = rotate(model, glm::radians(45.0f + i * 10), glm::vec3(1.0f, 1.0f, 0.0f));

		shader_lamp.SetMat4("uni_model", model);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	// ���
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
