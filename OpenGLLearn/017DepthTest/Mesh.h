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

void Mesh::DrawMesh(const Shader& shader, const Shader& shader_lamp, float posValue)
{
	// ��������Ԫ �κ�uniform���ò���һ��Ҫ�ŵ�����Ӧ��shader������֮��  --����ͬ��shader�л����У���һ��shader��ص���д�����ݻᶪʧ����
    // Ҳ����˵������shader֮����������shader_lamp��֮ǰ��shader���õľ���Ч�ˣ��������ֻ�ܷŵ���Ⱦѭ������ܷ�ѭ������
	glBindVertexArray(VAO); // draw������VAO�����Ķ���������    �ɴ���VBO EBO attrpoint�İ󶨲������������
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
		glBindTexture(GL_TEXTURE_2D, textures[i].id);  //Ƭ����ɫ������ݶ�Ӧ������Ԫ��ȡtexture_diffuse����ͼ����
	}
	cout << endl;
	mat4 model = mat4(1.0f);           
	model = scale(model, vec3(0.3f));
	shader.SetMat4("uni_model", model);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// ���
	glBindVertexArray(0);

	//glBindVertexArray(lampVAO);
	//shader_lamp.Use();
	//for (int i = 0; i < 4; i++)
	//{
	//	stringstream ss;
	//	ss << "pointLight[" << i << "].";
	//	string prefix = ss.str();

	//	vec3 lightPos = vec3(5 * cos(posValue + i * 10), 10.0f, 5 * sin(posValue + i * 10));

	//	mat4 model = mat4(1.0f);  // ��ʼ��Ϊ��λ�������
	//	model = scale(model, vec3(0.5f));
	//	model = translate(model, lightPos);
	//	model = rotate(model, radians(45.0f + i * 10), vec3(1.0f, 1.0f, 0.0f));

	//	shader_lamp.SetMat4("uni_model", model);

	//	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	//}

	//// ���
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
