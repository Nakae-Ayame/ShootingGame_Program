#include "ResourceManager.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <filesystem>
#include "Renderer.h" // for device/context
#include <iostream>

ResourceManager& ResourceManager::Get() 
{
    static ResourceManager inst;
    return inst;
}

std::future<std::shared_ptr<ModelCPU>> ResourceManager::LoadModelCPUAsync(const std::string& path) {
    // 非同期タスクを返す
    return std::async(std::launch::async, [this, path]() -> std::shared_ptr<ModelCPU> {
        // 既にキャッシュがあればそれを返す
        {
            std::lock_guard lk(m_mutex);
            auto it = m_cpuCache.find(path);
            if (it != m_cpuCache.end()) return it->second;
        }

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
        );

        auto model = std::make_shared<ModelCPU>();
        model->sourcePath = path;

        if (!scene || !scene->mRootNode) {
            OutputDebugStringA(("Assimp failed to load: " + path + "\n").c_str());
            return model;
        }

        // マテリアル読み取り
        for (unsigned m = 0; m < scene->mNumMaterials; ++m) {
            aiMaterial* aimat = scene->mMaterials[m];
            MaterialCPU mat;
            aiColor4D col;
            if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_DIFFUSE, col)) 
            {
                mat.diffuseColor = { col.r, col.g, col.b, col.a };
            }
            if (aimat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString texPath;
                if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
                {
                    mat.diffuseTexPath = texPath.C_Str();
                    // 相対パスを絶対に直す場合は modelDirectory を使ってここで直す
                }
            }
            model->materials.push_back(std::move(mat));
        }

        // メッシュ処理（簡易）
        std::function<void(aiNode*)> processNode;
        processNode = [&](aiNode* node) {
            for (unsigned i = 0; i < node->mNumMeshes; ++i)
            {
                aiMesh* am = scene->mMeshes[node->mMeshes[i]];
                MeshCPU mesh;
                mesh.materialIndex = am->mMaterialIndex;
                mesh.vertices.resize(am->mNumVertices);
                for (unsigned v = 0; v < am->mNumVertices; ++v)
                {
                    VERTEX_3D vv{};
                    vv.Position = { am->mVertices[v].x, am->mVertices[v].y, am->mVertices[v].z };
                    if (am->HasNormals()) vv.Normal = { am->mNormals[v].x, am->mNormals[v].y, am->mNormals[v].z };
                    if (am->mTextureCoords[0]) vv.TexCoord = { am->mTextureCoords[0][v].x, am->mTextureCoords[0][v].y };
                    // 色やボーンなど必要ならセット
                    mesh.vertices[v] = vv;
                }
                mesh.indices.reserve(am->mNumFaces * 3);
                for (unsigned f = 0; f < am->mNumFaces; ++f) 
                {
                    const aiFace& face = am->mFaces[f];
                    mesh.indices.insert(mesh.indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
                }
                model->meshes.push_back(std::move(mesh));
            }
            for (unsigned c = 0; c < node->mNumChildren; ++c) processNode(node->mChildren[c]);
            };

        processNode(scene->mRootNode);

        {
            std::lock_guard lk(m_mutex);
            m_cpuCache[path] = model;
        }
        OutputDebugStringA(("Loaded CPU model: " + path + "\n").c_str());
        return model;
        });
}

std::shared_ptr<ModelCPU> ResourceManager::GetCPU(const std::string& path) {
    std::lock_guard lk(m_mutex);
    auto it = m_cpuCache.find(path);
    return (it == m_cpuCache.end()) ? nullptr : it->second;
}

bool ResourceManager::HasCPU(const std::string& path) {
    std::lock_guard lk(m_mutex);
    return m_cpuCache.count(path) != 0;
}

std::shared_ptr<GPUModel> ResourceManager::CreateGPUFromCPU(const std::shared_ptr<ModelCPU>& cpu) {
    if (!cpu) return nullptr;
    std::lock_guard lk(m_mutex);
    if (m_gpuCache.count(cpu->sourcePath))
    {
        return m_gpuCache[cpu->sourcePath];
    }

    auto gpu = std::make_shared<GPUModel>();
    gpu->path = cpu->sourcePath;
    ID3D11Device* dev = Renderer::GetDevice();
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();

    // テクスチャは m_directory + cpu->materials[i].diffuseTexPath を組み立てて Load
    // ここでは簡潔に示す：CreateWICTextureFromFile を使う（main thread）
    for (auto& mat : cpu->materials) {
        MaterialGPU mg;
        mg.diffuseColor = mat.diffuseColor;
        if (!mat.diffuseTexPath.empty()) {
            std::wstring wpath; // convert to wstring
            int len = MultiByteToWideChar(CP_UTF8, 0, mat.diffuseTexPath.c_str(), -1, nullptr, 0);
            wpath.resize(len);
            MultiByteToWideChar(CP_UTF8, 0, mat.diffuseTexPath.c_str(), -1, &wpath[0], len);
            HRESULT hr = DirectX::CreateWICTextureFromFile(dev, ctx, wpath.c_str(), nullptr, mg.diffuseSRV.GetAddressOf());
            if (FAILED(hr)) {
                OutputDebugStringA(("CreateWICTextureFromFile failed for " + mat.diffuseTexPath + "\n").c_str());
            }
        }
        gpu->materials.push_back(std::move(mg));
    }

    for (auto& m : cpu->meshes) {
        MeshGPU mgpu;
        mgpu.materialIndex = m.materialIndex;
        mgpu.indexCount = (UINT)m.indices.size();
        // VB
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.ByteWidth = UINT(sizeof(VERTEX_3D) * m.vertices.size());
        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = m.vertices.data();
        dev->CreateBuffer(&bd, &sd, mgpu.vb.GetAddressOf());
        // IB
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.ByteWidth = UINT(sizeof(uint32_t) * m.indices.size());
        sd.pSysMem = m.indices.data();
        dev->CreateBuffer(&bd, &sd, mgpu.ib.GetAddressOf());

        gpu->meshes.push_back(std::move(mgpu));
    }

    m_gpuCache[cpu->path] = gpu;
    OutputDebugStringA(("Created GPUModel for " + cpu->path + "\n").c_str());
    return gpu;
}

std::shared_ptr<GPUModel> ResourceManager::GetGPU(const std::string& path) {
    std::lock_guard lk(m_mutex);
    auto it = m_gpuCache.find(path);
    return (it == m_gpuCache.end()) ? nullptr : it->second;
}

bool ResourceManager::HasGPU(const std::string& path) {
    std::lock_guard lk(m_mutex);
    return m_gpuCache.count(path) != 0;
}
