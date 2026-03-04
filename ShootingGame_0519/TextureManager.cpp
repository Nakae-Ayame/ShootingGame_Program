#ifdef _WIN32
#include <windows.h>
#endif
#include "TextureManager.h"
#include <WICTextureLoader.h>
#include "Renderer.h"

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> TextureManager::m_textures;

ID3D11ShaderResourceView* TextureManager::Load(const std::string& filepath)
{
    auto it = m_textures.find(filepath);
    if (it != m_textures.end())
    {
        return it->second.Get();
    }

    Microsoft::WRL::ComPtr<ID3D11Resource> res;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

    std::wstring wpath(filepath.begin(), filepath.end());

    HRESULT hr = DirectX::CreateWICTextureFromFile(Renderer::GetDevice(),
                                                   Renderer::GetDeviceContext(),
                                                   wpath.c_str(),
                                                   res.GetAddressOf(),
                                                   texture.GetAddressOf(),
                                                   0);

    if (FAILED(hr) || !texture)
    {
        return nullptr;
    }

    Renderer::GetDeviceContext()->GenerateMips(texture.Get());

    m_textures[filepath] = texture;

    /*Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
    res.As(&tex2D);

    D3D11_TEXTURE2D_DESC d{};
    tex2D->GetDesc(&d);

    char buf[256];
    sprintf_s(buf, "Texture MipLevels=%u, Format=%u, BindFlags=0x%08X\n", d.MipLevels, d.Format, d.BindFlags);
    OutputDebugStringA(buf);*/

    return texture.Get();
}
