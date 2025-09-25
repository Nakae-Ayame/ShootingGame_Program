#pragma once
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <DirectXMath.h> // optional: if you like SimpleMath use your own types

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT4;

struct VertexCPU 
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT4 Color;
    XMFLOAT2 TexCoord;    
};

struct MeshCPU 
{
    std::vector<VertexCPU> vertices;
    std::vector<uint32_t> indices;
    int materialIndex = -1;
};

struct MaterialCPU 
{
    std::string name;
    XMFLOAT4 diffuseColor = { 1,1,1,1 };
    std::string diffuseTexturePath; // full/relative path to image file
    // optionally: decoded pixel buffer (but large -> avoid; prefer load-to-GPU later)
};

struct ModelCPU 
{
    std::vector<MeshCPU> meshes;
    std::vector<MaterialCPU> materials;
    std::string sourcePath; // original file path
};
using ModelCPUPtr = std::shared_ptr<ModelCPU>;

