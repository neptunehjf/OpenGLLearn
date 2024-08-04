#pragma once

#include <vector>
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Shader.h"
#include "Mesh.h"
#include "namespace.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

class Model : public Mesh //���ü̳е��ĳ�Ա�ġ�������Ȩ�ޡ�Ϊpublic,ʵ��Ȩ�޻���Ҫ�Ը����Ȩ��Ϊ׼
{
public:
    Model(const string&& path)
    {
        loadModel(path);
    }
    void DrawModel(const Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].SetScale(m_scale);
            meshes[i].SetTranslate(m_translate);
            meshes[i].DrawMesh(shader);
        }    
    }
    void DeleteModel()
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DeleteMesh();
    }

private:
    vector<Mesh> meshes; // һ��model�ɶ��mesh��ɣ����糵��model�ɳ�ͷ�����ţ���̥��mesh���
    string directory;
    vector<Texture> texture_loaded; // ������ͼ�����ܴ�Ϊ���Ż����Ѿ����ع���texture�Ͳ�Ҫ�ظ�������

    void loadModel(string path);

    // ���ݸ���node��mesh indexȡ��scene���mesh��Դ��vertex normal texCoord face materialIndex��
    void processNode(aiNode* node, const aiScene* scene);
    // ��assimp��ʽ��mesh���ݽ����������Լ���mesh����
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    GLuint TextureFromFile(const string&& filePath, const string&& directory);
};


void Model::loadModel(string path)
{
    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate); //  | aiProcess_FlipUVs

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // ����node�ڵ��index���ҵ�scene���Ӧ��mesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // ��node�ڵ���еݹ����
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;

    // ��ȡvertices (position normal texCoord)
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
    {
        // mNumVertices�Ƕ������ mVertices��position����˼��������Ĳ��Ǻܺ�
        Vertex vertex = {vec3(0.0f), vec3(0.0f), vec2(0.0f)};
        // ����λ��
        vec3 position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        vertex.position = position;
        // ���㷨��
        vec3 normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        vertex.normal = normal;
        // ������������ Assimp����һ��ģ����һ�������������8����ͬ����������
        vec3 texCoord;
        if (mesh->mTextureCoords[0])  //��������Ƿ���İ������������꣨���ܲ�����һֱ��ˣ�
        {
            //ע��mTextureCoords�Ƕ�ά���飬��һ��ά�ȱ�ʾ����һ���������꣬��������ֻ���ĵ�һ��
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = texCoord;
        }
        vertices.push_back(vertex);
    }

    // ��ȡindices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // ��ȡMaterial
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // diffuse texure
    vector<Texture> diffuse = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse.begin(), diffuse.end());
    // specular texure
    vector<Texture> specular = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");;
    textures.insert(textures.end(), specular.begin(), specular.end());

    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (unsigned int j = 0; j < texture_loaded.size(); j++)
        {
            if (strcmp(texture_loaded[j].path.C_Str(), str.C_Str()) == 0) 
            {
                // �����Ѿ����ص���ͼ��ֱ�Ӹ���texture_loaded����ͼ���ݾ����ˣ�����Ҫ�ٴ�Ӳ�̼����ˡ�
                skip = true;
                textures.push_back(texture_loaded[j]);
                break;
            }
        }
        if (!skip)
        {
            // ��Ӳ�̼�����ͼ
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory.c_str());
            texture.type = typeName;
            texture.path = str;
            textures.push_back(texture);

            // Ϊ�˸���������Լ��Ӳ�̼��صĿ���
            texture_loaded.push_back(texture);
        }

    }
    return textures;
}

GLuint Model::TextureFromFile(const string&& filePath, const string&& directory)
{
    GLuint textureID = 0;
    // �����Դ�ռ䲢��GL_TEXTURE_2D����
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID); // �󶨲���Ҫô�Ƕ�Ҫô��д��������Ҫд
    // ����GL_TEXTURE_2D�Ļ��ƣ����˷�ʽ
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // ������ͼ��ת��Ϊ��������
    int width = 0, height = 0, channel = 0;
    string file = directory + "/" + filePath;
    unsigned char* data = stbi_load(file.c_str(), &width, &height, &channel, 0);

    GLenum format = 0;
    if (channel == 1)
        format = GL_RED;
    else if (channel == 3)
        format = GL_RGB;
    else if (channel == 4)
        format = GL_RGBA;

    if (data)
    {
        // ��ͼ���� �ڴ� -> �Դ�
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        // ���ɶ༶������ͼ
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture��" << endl;
    }
    // ���������Ѿ������Դ��ˣ�ɾ���ڴ��е���������
    stbi_image_free(data);

    return textureID;
}
