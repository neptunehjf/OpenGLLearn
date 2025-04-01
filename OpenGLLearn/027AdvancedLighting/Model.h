#pragma once

#include <vector>
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Shader.h"
#include "Mesh.h"
#include "common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

class Model : public Mesh // 设置继承到的成员的《最大访问权限》为public,实际权限还是要以父类的权限为准
                          // 継承されたメンバの《最大アクセス権限》をpublicに設定（実際の権限は親クラスのアクセス指定が優先）

{
public:
    Model()
    {
    }

    Model(const string&& path, const vector<mat4> & instMat4 = {})
    {
        this->instMat4 = instMat4;
        loadModel(path);
    }
    void DrawModel(const Shader& shader, bool bInst = false)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].SetScale(m_scale);
            meshes[i].SetTranslate(m_translate);
            meshes[i].SetRotate(m_rotateAngle, m_rotateAxis);
            meshes[i].DrawMesh(shader, GL_TRIANGLES, bInst);
        }    
    }

    void UniversalDrawModel(const Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].SetModel(m_model);
            meshes[i].UniversalDrawMesh(shader, GL_TRIANGLES);
        }
    }

    void DeleteModel()
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DeleteMesh();
    }
    vector<Mesh>& GetMeshes()
    {
        vector<Mesh>& _meshes = meshes; // 注意，如果想return引用的话，必须先先定义一个引用类型的局部变量，再return这个局部变量，否则return的不是引用类型
										// ローカル参照変数を介して参照を戻らなければならない
        return _meshes;
    }

private:
    vector<Mesh> meshes; // 一个model由多个mesh组成，比如车的model由车头，车门，轮胎等mesh组成
                         // モデルは複数のメッシュで構成される。例：車のモデルはボディ、ドア、タイヤなどのメッシュで構成
    string directory;
    vector<Texture> texture_loaded; // 加载贴图开销很大，为了优化，已经加载过的texture就不要重复加载了
                                    // テクスチャの読み込みは負荷が高いため、既に読み込んだテクスチャは重複して読み込まないように最適化
    vector<mat4> instMat4;

    void loadModel(string path);

    // 根据各个node的mesh index取出scene里的mesh资源（vertex normal texCoord face materialIndex）
    //  各ノードのメッシュインデックスからscene内のメッシュリソースを取得
    void processNode(aiNode* node, const aiScene* scene);
    // 把assimp格式的mesh数据解析成我们自己的mesh数据
    // Assimp形式のメッシュデータを自分のメッシュデータ形式に変換
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
    // 遍历node节点的mesh index，找到scene里对应的mesh
    //ノードのメッシュのインデックスを走査し、scene内の対応するメッシュを検索
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // 子node节点进行递归操作
    // 子ノードに対して再帰処理を実行
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
    // vertices (position normal texCoord)を取得
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
    {
        Vertex vertex = {vec3(0.0f), vec3(0.0f), vec2(0.0f)};
        // 頂点位置
        vec3 position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        vertex.position = position;
        // 頂点法線
        vec3 normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        vertex.normal = normal;
        // 顶点纹理坐标 Assimp允许一个模型在一个顶点上有最多8组不同的纹理坐标
        // 頂点テクスチャ座標（Assimpは1頂点あたり最大8つセットのテクスチャ座標をサポート）
        vec3 texCoord;
        if (mesh->mTextureCoords[0])
        {
            // 注意 mTextureCoords是二维数组，第一个维度表示是哪一组纹理坐标，我们这里只关心第一组
            // 注意: mTextureCoordsは2次元配列（第1次元がテクスチャ座標セット番号）。当処理では第1セットのみ使用
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = texCoord;
        }
        vertices.push_back(vertex);
    }

    // indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Material
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // diffuse texure
    vector<Texture> diffuse = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse.begin(), diffuse.end());
    // specular texure
    vector<Texture> specular = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");;
    textures.insert(textures.end(), specular.begin(), specular.end());
    // reflection texure
    vector<Texture> reflection = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");;
    textures.insert(textures.end(), reflection.begin(), reflection.end());

    return Mesh(vertices, indices, textures, instMat4);
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
                // 既に読み込み済みのテクスチャはスキップし、texture_loaded内のテクスチャデータを再利用。ディスクからの再読み込み不要
                skip = true;
                textures.push_back(texture_loaded[j]);
                break;
            }
        }
        if (!skip)
        {
            // 从硬盘加载贴图
            // ディスクからテクスチャを読み込み
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory.c_str());
            texture.type = typeName;
            texture.path = str;
            textures.push_back(texture);

            // 为了复用纹理，节约从硬盘加载的开销
            // テクスチャの再利用によるディスク読み込みオーバーヘッド削減のため
            texture_loaded.push_back(texture);
        }

    }
    return textures;
}

GLuint Model::TextureFromFile(const string&& filePath, const string&& directory)
{
    GLuint textureID = 0;
    // 申请显存空间并绑定GL_TEXTURE_2D对象
    // VRAM領域確保し、GL_TEXTURE_2Dオブジェクトをバインド
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID); 
    // 设置GL_TEXTURE_2D的环绕，过滤方式
    // GL_TEXTURE_2Dのラップモードとフィルタリング設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载贴图，转换为数据
    // テクスチャ読み込み、データに変換
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
        // 贴图数据传入显存
        /* テクスチャデータをVRAMに転送する */
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        // 生成多级渐进贴图
        // ミップマップを生成する  
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture！" << endl;
    }
    // 数据已经传给显存了，删除内存中的数据
    // メモリ上のデータ削除（VRAMに転送完了後）
    stbi_image_free(data);

    // 读写结束之后一定要记得解绑，否则会在预想外的地方被写入值
	// // 読み取りと書き込みの後に必ずアンバインドすること。そうしないと、値が予期しない場所に書き込まれます。
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}