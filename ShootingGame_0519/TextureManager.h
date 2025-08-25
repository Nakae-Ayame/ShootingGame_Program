// TextureManager.h
#pragma once
#include <string>
#include <unordered_map>
#include <wrl/client.h>
#include <d3d11.h>

class TextureManager
{
public:
    static ID3D11ShaderResourceView* Load(const std::string& filepath);

private:
    static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
};
