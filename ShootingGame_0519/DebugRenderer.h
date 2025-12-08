#pragma once
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include <SimpleMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

struct DebugLineVertex { Vector3 pos; Vector4 color; };

class DebugRenderer
{
public:
    DebugRenderer() = default;
    ~DebugRenderer() = default;

    static DebugRenderer& Get()
    {
        static DebugRenderer instance;
        return instance;
    }

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
        const wchar_t* vsPath = L"DebugLineVS.cso",
        const wchar_t* psPath = L"DebugLinePS.cso");

    void AddLine(const Vector3& a, const Vector3& b, const Vector4& color);
    void AddBox(const Vector3& center, const Vector3& size, const Matrix& rot, const Vector4& color);
    void AddSphere(const Vector3& center, float radius, const Vector4& color, int segments = 24);

    // view/proj ÇñàÉtÉåÅ[ÉÄó^Ç¶Çƒï`âÊ
    void Draw(const Matrix& view, const Matrix& proj);
    void Clear();

private:
    void EnsureVertexBufferCapacity(size_t vertexCountNeeded);

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11VertexShader> m_vs;
    ComPtr<ID3D11PixelShader> m_ps;
    ComPtr<ID3D11Buffer> m_cbViewProj;
    ComPtr<ID3D11BlendState> m_blendAlpha;

    std::vector<DebugLineVertex> m_lineVerts;
    size_t m_vbCapacity = 0;
};
