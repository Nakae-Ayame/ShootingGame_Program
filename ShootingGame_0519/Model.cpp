#include "Model.h"
#include <WICTextureLoader.h> 
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "renderer.h"
#include <filesystem>
#include <iostream>
//------------------------------------------------------------
//モデルのファイルを読み込む関数(引数：モデルファイルがある所のパス)
//------------------------------------------------------------
bool Model::LoadFromFile(const std::string& path)
{
    // ファイルパスからディレクトリ部分を抜き出して保持
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        modelDirectory_ = path.substr(0, pos + 1);
    }
    else
    {
        modelDirectory_.clear();
    }

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace
    );

    // シーンが読み込めなかった場合は false を返す
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        return false;
    }

    //---------------------------------------------
    // ✅ ここでシーン内のすべてのマテリアルを処理
    //---------------------------------------------
    materials_.clear(); // 念のため初期化

    for (UINT i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aimat = scene->mMaterials[i];

        Material mat;

        // マテリアル名（省略可だがログなどで便利）
        aiString name;
        if (aimat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
        {
            mat.name = name.C_Str();
        }

        // Diffuseカラー
        aiColor4D col;
        if (aimat->Get(AI_MATKEY_COLOR_DIFFUSE, col) == AI_SUCCESS)
        {
            mat.diffuseColor = Vector4(col.r, col.g, col.b, col.a);
        }

        // Diffuseテクスチャパスの取得
        aiString texPath;
        if (aimat->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
            aimat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
        {
            mat.texPath_Diffuse = texPath.C_Str();

            // テクスチャのフルパスを構成（相対 or 絶対）
            std::string fullPath = texPath.C_Str();
            if (!std::filesystem::path(fullPath).is_absolute())
            {
                fullPath = modelDirectory_ + fullPath;
            }

            // 文字列変換（UTF8 → UTF16）
            int len = MultiByteToWideChar(CP_UTF8, 0, fullPath.c_str(), -1, nullptr, 0);
            std::wstring wpath(len, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, fullPath.c_str(), -1, &wpath[0], len);

            // テクスチャ読み込み
            HRESULT hr = DirectX::CreateWICTextureFromFile(
                Renderer::GetDevice(), Renderer::GetDeviceContext(),
                wpath.c_str(), nullptr, mat.texSRV_Diffuse.GetAddressOf());

            if (FAILED(hr))
            {
                OutputDebugStringA(("テクスチャ読み込み失敗: " + fullPath + "\n").c_str());
            }
        }

        // 読み込んだマテリアルを保存
        materials_.push_back(std::move(mat));
    }

    //---------------------------------------------
    // メッシュ構造（ノード）を再帰的に処理
    //---------------------------------------------
    ProcessNode(scene->mRootNode, scene);

    return true;
}

//------------------------------------------------------------
//モデルのテクスチャを読み込む関数
//------------------------------------------------------------
Material LoadMaterial(aiMaterial* aimat)
{
    Material mat;

    // 名前
    aiString name;
    if (aimat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
    {
        mat.name = name.C_Str();
    }

    // Diffuse色
    aiColor4D col;
    if (aimat->Get(AI_MATKEY_COLOR_DIFFUSE, col) == AI_SUCCESS)
    {
        mat.diffuseColor = Vector4(col.r, col.g, col.b, col.a);
    }

    // テクスチャパス
    aiString path;
    if (aimat->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        aimat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
    {
        mat.texPath_Diffuse = path.C_Str();
    }

    return mat;
}


void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    // ノード内の各メッシュを処理
    for (UINT i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* am = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(ProcessMesh(am, scene));
    }
    // 子ノードも再帰
    for (UINT i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

MeshPart Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    MeshPart part;

    // マテリアルインデックスを保存（描画時に使う）
    part.materialIndex = mesh->mMaterialIndex;

    // 頂点データ生成
    std::vector<VERTEX_3D> verts(mesh->mNumVertices);
    std::vector<UINT> indices;
    indices.reserve(mesh->mNumFaces * 3);

    // マテリアル色取得
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    aiColor4D c;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, c);
    Vector4 diffuseColor(c.r, c.g, c.b, c.a);

    for (UINT i = 0; i < mesh->mNumVertices; ++i)
    {
        verts[i].Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        verts[i].Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if (mesh->mTextureCoords[0])
        {
            verts[i].TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        else
        {
            verts[i].TexCoord = { 0, 0 };
        }

        // ここでは色だけ使う（テクスチャは materials_ 側で管理）
        verts[i].Diffuse = diffuseColor;
    }

    // インデックス
    for (UINT f = 0; f < mesh->mNumFaces; ++f)
    {
        const aiFace& face = mesh->mFaces[f];
        indices.insert(indices.end(), face.mIndices, face.mIndices + 3);
    }

    auto device = Renderer::GetDevice();
    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA sd{};

    // 頂点バッファ生成
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = UINT(sizeof(VERTEX_3D) * verts.size());
    sd.pSysMem = verts.data();
    device->CreateBuffer(&bd, &sd, part.vb.GetAddressOf());

    // インデックスバッファ生成
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = UINT(sizeof(UINT) * indices.size());
    sd.pSysMem = indices.data();
    device->CreateBuffer(&bd, &sd, part.ib.GetAddressOf());

    part.indexCount = UINT(indices.size());

    // ✅ テクスチャの読み込みはしない（マテリアルに任せる）
    return part;
}

void Model::Draw(const DirectX::XMMATRIX& worldMatrix)
{
    //std::cout << "[Model] Draw開始\n";

    // ここでワールド行列をセット（あなたのRendererの設計に沿って）
    Matrix4x4 mat;
    XMStoreFloat4x4(&mat, worldMatrix);  // worldMatrix(XMMATRIX) → Matrix4x4 へ変換
    Renderer::SetWorldMatrix(&mat);

    for (const MeshPart& part : meshes_)
    {
        //std::cout << "[Model] MeshPart描画 indexCount=" << part.indexCount << "\n";

        const Material& mat = materials_[part.materialIndex];

        if (mat.texSRV_Diffuse)
        {
            Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, mat.texSRV_Diffuse.GetAddressOf());
        }

        UINT stride = sizeof(VERTEX_3D);
        UINT offset = 0;
        Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, part.vb.GetAddressOf(), &stride, &offset);
        Renderer::GetDeviceContext()->IASetIndexBuffer(part.ib.Get(), DXGI_FORMAT_R32_UINT, 0);
        Renderer::GetDeviceContext()->DrawIndexed(part.indexCount, 0, 0);
    }
}