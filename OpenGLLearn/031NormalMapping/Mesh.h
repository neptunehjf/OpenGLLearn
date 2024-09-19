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
	Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, 
		 const vector<Texture>& textures, const vector<mat4>& instMat4 = {});
	Mesh(const vector<float>& vertices, const vector<uint>& indices, const vector<uint>& parse, 
		 const vector<Texture>& textures, const vector<vec2>& instanceArray);
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

protected:  //ֻ�����������
	vec3 m_scale;
	vec3 m_translate;
	float m_rotateAngle;
	vec3 m_rotateAxis;
	mat4 m_model;

private:
	void SetupMesh(bool bInst = false);
	//void UniversalSetupMesh();

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

//Mesh::Mesh(const vector<float>& vertices, const vector<uint>& indices, const vector<uint>& parse,
//		   const vector<Texture>& textures, const vector<vec2>& instanceArray)
//{
//	this->u_vertices = vertices;
//	this->u_indices = indices;
//	this->parse = parse;
//	this->textures = textures;
//	this->instanceArray = instanceArray;
//
//	VAO = 0;
//	VBO = 0;
//	EBO = 0;
//	VBO_Instances = 0;
//
//	m_scale = vec3(1.0f);
//	m_translate = vec3(0.0f);
//	m_rotateAngle = 0.0f;
//	m_rotateAxis = vec3(1.0, 1.0, 1.0);
//	m_model = mat4(1.0f);
//
//	UniversalSetupMesh();
//}

Mesh::~Mesh()
{
}

void Mesh::SetupMesh(bool bInst)
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

	if (bInst)
		SetInstMat4();

	// ���
	glBindVertexArray(0);// �ر�VAO������

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//��Ŀ����GL_ELEMENT_ARRAY_BUFFER��ʱ��VAO�ᴢ��glBindBuffer�ĺ������á���Ҳ��ζ����Ҳ�ᴢ������ã�����ȷ����û���ڽ��VAO֮ǰ����������黺�壬��������û�����EBO������
	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//void Mesh::UniversalSetupMesh()
//{
//	// ���Դ�VAO������ shader�Ķ�������
//	glGenVertexArrays(1, &VAO);
//	glBindVertexArray(VAO); // VBO glVertexAttribPointer ������VAO������д
//
//	// �洢�������ݵ��Դ�VBO
//	glGenBuffers(1, &VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * u_vertices.size(), &u_vertices[0], GL_STATIC_DRAW);
//
//	// �洢�±����ݵ��Դ�EBO
//	glGenBuffers(1, &EBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * u_indices.size(), &u_indices[0], GL_STATIC_DRAW);
//
//	// ���嶥�����ԵĽ�����ʽ
//	uint stride = 0;
//	for (uint i = 0; i < parse.size(); i++)
//	{
//		stride += parse[i];
//	}
//	uint offset = 0;
//
//	for (uint i = 0; i < parse.size(); i++)
//	{
//		glVertexAttribPointer(i, parse[i], GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * offset));
//		glEnableVertexAttribArray(i);
//		offset += parse[i];
//	}
//
//	/**************************** ʵ�������� ****************************/
//	// ��ΪEBOֻ��ָ�������������˳���ǵ������ڵģ�����EBO���ڼ䲻��Ӱ�쵽 VBO_Instances������VBO��
//	// VBO���ڼ������Ӱ��VBO_Instances����ΪVBO �� VBO_Instancesƽ�����е�
//	// ����ֱ�ӽ��Ű�VBO_Instances���ɣ�����ʵ��������ͺ�layout location2��Ӧ��
//	// �洢ʵ�������鵽�Դ�VBO
//	glGenBuffers(1, &VBO_Instances);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO_Instances);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * instanceArray.size(), &instanceArray[0], GL_STATIC_DRAW);
//
//	// parse.size()���ö�Ӧʵ���������layout location 
//	int componetNum = sizeof(instanceArray[0]) / sizeof(GL_FLOAT);
//	glVertexAttribPointer(parse.size(), componetNum, GL_FLOAT, GL_FALSE, sizeof(float) * componetNum, (void*)(sizeof(float) * 0));
//	glEnableVertexAttribArray(parse.size());
//	// ָ��location2 ÿ��Ⱦ1��ʵ������1��instanceArray���ڶ���������0�Ļ�����û���ã�����ÿ��Ⱦһ���������1��ʵ�������ˣ����bug
//	glVertexAttribDivisor(2, 1);
//	
//	// ���
//	glBindVertexArray(0);// �ر�VAO������
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	//��Ŀ����GL_ELEMENT_ARRAY_BUFFER��ʱ��VAO�ᴢ��glBindBuffer�ĺ������á���Ҳ��ζ����Ҳ�ᴢ������ã�����ȷ����û���ڽ��VAO֮ǰ����������黺�壬��������û�����EBO������
//	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//}

//void Mesh::UniversalDrawMesh(const Shader& shader, GLuint element)
//{
//	// ��������Ԫ �κ�uniform���ò���һ��Ҫ�ŵ�����Ӧ��shader������֮��  --����ͬ��shader�л����У���һ��shader��ص���д�����ݻᶪʧ����
//	// Ҳ����˵������shader֮����������shader_lamp��֮ǰ��shader���õľ���Ч�ˣ��������ֻ�ܷŵ���Ⱦѭ������ܷ�ѭ������
//	glBindVertexArray(VAO); // draw������VAO�����Ķ���������    �ɴ���VBO EBO attrpoint�İ󶨲������������
//	shader.Use();
//	GLuint diffuseN = 0;
//	GLuint specularN = 0;
//	GLuint reflectionN = 0;
//	GLuint cubemapN = 0;
//	string type;
//
//	for (uint i = 0; i < textures.size(); i++)
//	{
//		uint samplerID = i + 1; // ��1����Ϊ�뱣��GL_TEXTURE0��ֻҪ������GL_TEXTURE0����Ϊ������û�ж���shader
//		type = textures[i].type;
//		if (type == "texture_diffuse")
//		{
//			//cout << "texture_diffuse" << endl;
//			diffuseN++;
//			shader.SetInt("material." + type + to_string(diffuseN), samplerID);   // ���������һ��draw�ж����ͼҪ��ô�㣬���������ұ���
//		}
//		else if (type == "texture_specular")
//		{
//			//cout << "texture_specular" << endl;
//			specularN++;
//			shader.SetInt("material." + type + to_string(specularN), samplerID);
//		}
//		else if (type == "texture_reflection")
//		{
//			//cout << "texture_reflection" << endl;
//			reflectionN++;
//			shader.SetInt("material." + type + to_string(reflectionN), samplerID);
//		}
//		else if (type == "texture_cubemap")
//		{
//			//cout << "texture_cubemap" << endl;
//			cubemapN++;
//			shader.SetInt(type + to_string(cubemapN), samplerID);
//		}
//
//		//cout << i << endl << endl;
//		glActiveTexture(GL_TEXTURE0 + samplerID);
//		if (type == "texture_cubemap")
//		{
//			glBindTexture(GL_TEXTURE_CUBE_MAP, textures[i].id);
//		}
//		else
//		{
//			glBindTexture(GL_TEXTURE_2D, textures[i].id);
//		}
//	}
//	//cout << "***********************************" << endl;
//
//	shader.SetMat4("uni_model", m_model);
//
//	glDrawElements(element, indices.size(), GL_UNSIGNED_INT, 0);
//	//glDrawElementsInstanced(element, u_indices.size(), GL_UNSIGNED_INT, 0, 100);
//
//	// ���
//	diffuseN = 0;
//	specularN = 0;
//	reflectionN = 0;
//	cubemapN = 0;
//	for (uint i = 0; i < textures.size(); i++)
//	{
//		uint samplerID = i + 1; // ��1����Ϊ�뱣��GL_TEXTURE0��ֻҪ������GL_TEXTURE0����Ϊ������û�ж���shader
//		type = textures[i].type;
//		if (type == "texture_diffuse")
//		{
//			//cout << "texture_diffuse" << endl;
//			diffuseN++;
//			shader.SetInt("material." + type + to_string(diffuseN), samplerID);   // ���������һ��draw�ж����ͼҪ��ô�㣬���������ұ���
//		}
//		else if (type == "texture_specular")
//		{
//			//cout << "texture_specular" << endl;
//			specularN++;
//			shader.SetInt("material." + type + to_string(specularN), samplerID);
//		}
//		else if (type == "texture_reflection")
//		{
//			//cout << "texture_reflection" << endl;
//			reflectionN++;
//			shader.SetInt("material." + type + to_string(reflectionN), samplerID);
//		}
//		else if (type == "texture_cubemap")
//		{
//			//cout << "texture_cubemap" << endl;
//			cubemapN++;
//			shader.SetInt(type + to_string(cubemapN), samplerID);
//		}
//
//		//cout << i << endl << endl;
//		glActiveTexture(GL_TEXTURE0 + samplerID);
//		if (type == "texture_cubemap")
//		{
//			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//		}
//		else
//		{
//			glBindTexture(GL_TEXTURE_2D, 0);
//		}
//	}
//
//	glBindVertexArray(0);
//}

void Mesh::DrawMesh(const Shader& shader, GLuint element, bool bInst)
{
	// ��������Ԫ �κ�uniform���ò���һ��Ҫ�ŵ�����Ӧ��shader������֮��  --����ͬ��shader�л����У���һ��shader��ص���д�����ݻᶪʧ����
    // Ҳ����˵������shader֮����������shader_lamp��֮ǰ��shader���õľ���Ч�ˣ��������ֻ�ܷŵ���Ⱦѭ������ܷ�ѭ������
	glBindVertexArray(VAO); // draw������VAO�����Ķ���������    �ɴ���VBO EBO attrpoint�İ󶨲������������
	shader.Use();
	uint diffuseN = 0;
	uint specularN = 0;
	uint reflectionN = 0;
	uint cubemapN = 0;
	uint normalN = 0;
	string type;

	for (uint i = 0; i < textures.size(); i++)
	{
		type = textures[i].type;
		shader.SetBool("bNormalMap", false);
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
		else if (type == "texture_normal")
		{
			//cout << "texture_normal" << endl;
			normalN++;
			shader.SetInt("material." + type + to_string(normalN), i);
			if (bEnableNormalMap)
				shader.SetBool("bNormalMap", true);
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
		// ����Ҫset uniform ��model��Ϊʵ�����������Դ���
		glDrawElementsInstanced(element, indices.size(), GL_UNSIGNED_INT, 0, ROCK_NUM);
	}

	// ���
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
		else if (type == "texture_normal")
		{
			//cout << "texture_normal" << endl;
			normalN++;
			shader.SetInt(type + to_string(normalN), i);
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
	/**************************** ʵ�������� ****************************/
// ��ΪEBOֻ��ָ�������������˳���ǵ������ڵģ�����EBO���ڼ䲻��Ӱ�쵽 VBO_Instances������VBO��
// VBO���ڼ������Ӱ��VBO_Instances����ΪVBO �� VBO_Instancesƽ�����е�
// ����ֱ�ӽ��Ű�VBO_Instances���ɣ�����ʵ��������ͺ�layout location2��Ӧ��
// �洢ʵ�������鵽�Դ�VBO
// ָ��location2 ÿ��Ⱦ1��ʵ������1��instanceArray���ڶ���������0�Ļ�����û���ã�����ÿ��Ⱦһ���������1��ʵ�������ˣ����bug

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