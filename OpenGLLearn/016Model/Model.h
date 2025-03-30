﻿#pragma once

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

class Model
{
public:
    Model(const string&& path)
    {
        loadModel(path);
    }
    void DrawModel(const Shader& shader, const Shader& shader_lamp, float posValue)
    {

        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DrawMesh(shader, shader_lamp, posValue);

        cout << "texture_loaded " << texture_loaded.size() << endl;
    }
    void DeleteModel()
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DeleteMesh();
    }

private:
    vector<Mesh> meshes; // 一个model由多个mesh组成，比如车的model由车头，车门，轮胎等mesh组成
    string directory;
    vector<Texture> texture_loaded; // 加载贴图开销很大，为了优化，已经加载过的texture就不要重复加载了

    void loadModel(string path);

    // 根据各个node的mesh index取出scene里的mesh资源（vertex normal texCoord face materialIndex）
    void processNode(aiNode* node, const aiScene* scene);
    // 把assimp格式的mesh数据解析成我们自己的mesh数据
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    GLuint TextureFromFile(const string&& filePath, const string&& directory);
};


void Model::loadModel(string path)
{
    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

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
    // 遍历node节点的index，找到scene里对应的mesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // 子node节点进行递归操作
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

    // 获取vertices (position normal texCoord)
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
    {
        // mNumVertices是顶点个数 mVertices是position的意思，名字起的不是很好
        Vertex vertex = {vec3(0.0f), vec3(0.0f), vec2(0.0f)};
        // 顶点位置
        vec3 position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        vertex.position = position;
        // 顶点法线
        vec3 normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        vertex.normal = normal;
        // 顶点纹理坐标 Assimp允许一个模型在一个顶点上有最多8个不同的纹理坐标
        vec3 texCoord;
        if (mesh->mTextureCoords[0])  //检查网格是否真的包含了纹理坐标（可能并不会一直如此）
        {
            //注意mTextureCoords是二维数组，第一个维度表示是哪一组纹理坐标，我们这里只关心第一组
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = texCoord;
        }
        vertices.push_back(vertex);
    }

    // 获取indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 获取Material
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // diffuse texure
    vector<Texture> diffuse = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse.begin(), diffuse.end());
    // specular texure
    vector<Texture> specular = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");;
    textures.insert(textures.end(), specular.begin(), specular.end());

    cout << endl;

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
                // 跳过已经加载的贴图，直接复用texture_loaded的贴图数据就行了，不需要再从硬盘加载了。
                cout << "skip" << endl;
                skip = true;
                textures.push_back(texture_loaded[j]);
                break;
            }
        }
        if (!skip)
        {
            // 从硬盘加载贴图
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory.c_str());
            texture.type = typeName;
            texture.path = str;
            textures.push_back(texture);

            // 为了复用纹理，节约从硬盘加载的开销
            texture_loaded.push_back(texture);
        }

    }
    cout << "Load " + typeName + " texture count " << textures.size() << endl;
    return textures;
}

GLuint Model::TextureFromFile(const string&& filePath, const string&& directory)
{
    GLuint textureID = 0;
    // 申请显存空间并绑定GL_TEXTURE_2D对象
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID); // 绑定操作要么是读要么是写，这里是要写
    // 设置GL_TEXTURE_2D的环绕，过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载贴图，转换为像素数据
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

    // debug code
 
    //printf("**********************************************************\n");
    //for (int i = 1; i <= width * height; i++)
    //{
    //	printf("%02x ", data[i]);
    //	if (i % width == 0)
    //		printf("\n");
    //}
    //printf("**********************************************************\n"); 

    if (data)
    {
        // 贴图数据 内存 -> 显存
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        // 生成多级渐进贴图
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture！" << endl;
    }
    // 像素数据已经传给显存了，删除内存中的像素数据
    stbi_image_free(data);

    return textureID;
}

