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

//��������
#pragma pack(1)
struct Vertex
{
	vec3 position; //λ��
	vec3 normal;   //����
	vec2 texCoord; //��������
};
#pragma pack()

//��ͼ����
#pragma pack(1)
struct Texture
{
	unsigned int id; //��ͼid
	string type;     //��ͼ���ͣ����� ��������ͼ ���� �߹���ͼ
	aiString path;     //��ͼ·��
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

protected:  //ֻ�����������
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//��Ŀ����GL_ELEMENT_ARRAY_BUFFER��ʱ��VAO�ᴢ��glBindBuffer�ĺ������á���Ҳ��ζ����Ҳ�ᴢ������ã�����ȷ����û���ڽ��VAO֮ǰ����������黺�壬��������û�����EBO������
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(const Shader& shader)
{
	// ��������Ԫ �κ�uniform���ò���һ��Ҫ�ŵ�����Ӧ��shader������֮��  --����ͬ��shader�л����У���һ��shader��ص���д�����ݻᶪʧ����
    // Ҳ����˵������shader֮����������shader_lamp��֮ǰ��shader���õľ���Ч�ˣ��������ֻ�ܷŵ���Ⱦѭ������ܷ�ѭ������
	glBindVertexArray(VAO); // draw������VAO�����Ķ���������    �ɴ���VBO EBO attrpoint�İ󶨲������������
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
			shader.SetInt("material." + type + to_string(diffuseN), i);   // ���������һ��draw�ж����ͼҪ��ô�㣬���������ұ���
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

	// ���
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