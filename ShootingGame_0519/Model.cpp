#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "renderer.h"

bool Model::LoadFromFile(const std::string& path)
{

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        // 読み込み失敗
        return false;
    }
    // ルートノードから再帰処理
    ProcessNode(scene->mRootNode, scene);
    return true;

}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    // ノード内の各メッシュを処理
    for (UINT i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* am = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(ProcessMesh(am));
    }
    // 子ノードも再帰
    for (UINT i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

MeshPart Model::ProcessMesh(aiMesh* mesh)
{
    std::vector<VERTEX_3D> verts(mesh->mNumVertices);
    std::vector<UINT> indices;
    indices.reserve(mesh->mNumFaces * 3);

    // 頂点情報を転送
    for (UINT i = 0; i < mesh->mNumVertices; ++i)
    {
        verts[i].Position = { mesh->mVertices[i].x,
                              mesh->mVertices[i].y,
                              mesh->mVertices[i].z };
        verts[i].Normal = { mesh->mNormals[i].x,
                              mesh->mNormals[i].y,
                              mesh->mNormals[i].z };
        if (mesh->mTextureCoords[0]) {
            verts[i].TexCoord = { mesh->mTextureCoords[0][i].x,
                                  mesh->mTextureCoords[0][i].y };
        }
        verts[i].Diffuse = { 1,1,1,1 }; // デフォルト白
    }
    // インデックス情報を展開 (三角形のみ)
    for (UINT f = 0; f < mesh->mNumFaces; ++f)
    {
        aiFace& face = mesh->mFaces[f];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    MeshPart part;
    auto device = Renderer::GetDevice();
    D3D11_BUFFER_DESC bd{};
    D3D11_SUBRESOURCE_DATA sd{};

    // 頂点バッファ作成
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = UINT(sizeof(VERTEX_3D) * verts.size());
    sd.pSysMem = verts.data();
    device->CreateBuffer(&bd, &sd, part.vb.GetAddressOf());

    // インデックスバッファ作成
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = UINT(sizeof(UINT) * indices.size());
    sd.pSysMem = indices.data();
    device->CreateBuffer(&bd, &sd, part.ib.GetAddressOf());

    part.indexCount = UINT(indices.size());
    return part;
}

void Model::Draw(const SRT& transform)
{
    // ワールド行列を更新
    Matrix4x4 world = transform.GetMatrix().Transpose();
    Renderer::GetDeviceContext()->UpdateSubresource(
        Renderer::GetWorldBuffer(), 0, nullptr, &world, 0, 0);

    auto dc = Renderer::GetDeviceContext();
    UINT stride = sizeof(VERTEX_3D), offset = 0;
    for (auto& mesh : meshes_)
    {
        dc->IASetVertexBuffers(0, 1, mesh.vb.GetAddressOf(), &stride, &offset);
        dc->IASetIndexBuffer(mesh.ib.Get(), DXGI_FORMAT_R32_UINT, 0);
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        dc->DrawIndexed(mesh.indexCount, 0, 0);
    }
}
