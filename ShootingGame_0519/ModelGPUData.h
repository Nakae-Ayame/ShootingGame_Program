#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <vector>
#include <memory>

using Microsoft::WRL::ComPtr;

struct MeshGPU
{
    ComPtr<ID3D11Buffer> vb;
    ComPtr<ID3D11Buffer> ib;
    UINT indexCount = 0;
    int materialIndex = -1;
};

struct MaterialGPU
{
    ComPtr<ID3D11ShaderResourceView> srvDiffuse;
    // other maps...
    float diffuseColor[4] = { 1,1,1,1 };
};

struct GPUModel
{
    std::vector<MeshGPU> meshes;
    std::vector<MaterialGPU> materials;
    // optional: bounding box, LODs, etc.
};
using GPUModelPtr = std::shared_ptr<GPUModel>;

