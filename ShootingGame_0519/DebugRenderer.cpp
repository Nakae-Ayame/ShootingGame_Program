#include "DebugRenderer.h"
#include <d3dcompiler.h>
#include <cassert>
#include <fstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace
{
    // 16バイトアラインされた行列（転置して送る）
    struct CBViewProj
    {
        XMMATRIX viewProj;
    };

    // CSO を読み込むユーティリティ
    std::vector<char> LoadFileBinary(const wchar_t* path)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) return {};
        return std::vector<char>((std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());
    }
}

void DebugRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
    const wchar_t* vsPath, const wchar_t* psPath)
{
    assert(device && context);
    m_device = device;
    m_context = context;

    // --- シェーダ読み込み（CSO推奨） ---
    auto vsBin = LoadFileBinary(vsPath);
    auto psBin = LoadFileBinary(psPath);
    if (vsBin.empty() || psBin.empty())
    {
        // 必要に応じて例外 or ログ
        assert(false && "Failed to load DebugRenderer shaders CSO.");
        return;
    }
    HRESULT hr = S_OK;

    // VS
    hr = m_device->CreateVertexShader(vsBin.data(), vsBin.size(), nullptr, m_vs.GetAddressOf());
    assert(SUCCEEDED(hr));

    // PS
    hr = m_device->CreatePixelShader(psBin.data(), psBin.size(), nullptr, m_ps.GetAddressOf());
    assert(SUCCEEDED(hr));

    // 入力レイアウト（float3 POSITION, float4 COLOR）
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = m_device->CreateInputLayout(layout, _countof(layout),
        vsBin.data(), vsBin.size(), m_inputLayout.GetAddressOf());
    assert(SUCCEEDED(hr));

    // 定数バッファ（ViewProj）
    D3D11_BUFFER_DESC cbd = {};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(CBViewProj);
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.CPUAccessFlags = 0;
    hr = m_device->CreateBuffer(&cbd, nullptr, m_cbViewProj.GetAddressOf());
    assert(SUCCEEDED(hr));

    // 動的頂点バッファ（最初は小さく確保→必要に応じて拡張）
    m_vbCapacity = 1024; // 頂点数（=512本の線）くらいから
    D3D11_BUFFER_DESC vbd = {};
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = UINT(m_vbCapacity * sizeof(DebugLineVertex));
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = m_device->CreateBuffer(&vbd, nullptr, m_vertexBuffer.GetAddressOf());
    assert(SUCCEEDED(hr));

    // アルファブレンド（線色のαを活かす）
    D3D11_BLEND_DESC bd = {};
    bd.AlphaToCoverageEnable = FALSE;
    bd.IndependentBlendEnable = FALSE;
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = m_device->CreateBlendState(&bd, m_blendAlpha.GetAddressOf());
    assert(SUCCEEDED(hr));
}

void DebugRenderer::EnsureVertexBufferCapacity(size_t vertexCountNeeded)
{
    if (vertexCountNeeded <= m_vbCapacity) return;

    // 2倍法で拡張
    size_t newCapacity = m_vbCapacity;
    while (newCapacity < vertexCountNeeded) newCapacity *= 2;

    D3D11_BUFFER_DESC vbd = {};
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = UINT(newCapacity * sizeof(DebugLineVertex));
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Buffer> newVB;
    HRESULT hr = m_device->CreateBuffer(&vbd, nullptr, newVB.GetAddressOf());
    assert(SUCCEEDED(hr));

    m_vertexBuffer = newVB;
    m_vbCapacity = newCapacity;
}

void DebugRenderer::AddLine(const Vector3& a, const Vector3& b, const Vector4& color)
{
    m_lineVerts.push_back({ a, color });
    m_lineVerts.push_back({ b, color });
}

void DebugRenderer::AddBox(const Vector3& center, const Vector3& size, const Matrix& rot, const Vector4& color)
{
    Vector3 h = size * 0.5f;

    Vector3 local[8] =
    {
        { -h.x, -h.y, -h.z },
        { -h.x,  h.y, -h.z },
        {  h.x,  h.y, -h.z },
        {  h.x, -h.y, -h.z },
        { -h.x, -h.y,  h.z },
        { -h.x,  h.y,  h.z },
        {  h.x,  h.y,  h.z },
        {  h.x, -h.y,  h.z },
    };
    Vector3 w[8];
    for (int i = 0; i < 8; ++i)
        w[i] = Vector3::Transform(local[i], rot) + center;

    const int e[12][2] =
    {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for (int i = 0; i < 12; ++i)
    {
        AddLine(w[e[i][0]], w[e[i][1]], color);
    }
}

void DebugRenderer::Draw(const Matrix& view, const Matrix& proj)
{
    if (m_lineVerts.empty()) return;

    // --- Save GPU state that we'll overwrite ---
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ID3D11InputLayout* prevIL = nullptr;
    ID3D11Buffer* prevVSCB[4] = { nullptr, nullptr, nullptr, nullptr }; // adjust count if you need more slots
    ID3D11Buffer* prevPSCB[4] = { nullptr, nullptr, nullptr, nullptr };
    ID3D11BlendState* prevBlend = nullptr;
    FLOAT               prevBlendFactor[4] = { 0,0,0,0 };
    UINT                prevSampleMask = 0xFFFFFFFF;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* prevVBs[1] = { nullptr };
    UINT                prevStrides[1] = { 0 };
    UINT                prevOffsets[1] = { 0 };

    // Save common pipeline state
    m_context->VSGetShader(&prevVS, nullptr, nullptr);
    m_context->PSGetShader(&prevPS, nullptr, nullptr);
    m_context->IAGetInputLayout(&prevIL);
    m_context->VSGetConstantBuffers(0, _countof(prevVSCB), prevVSCB);
    m_context->PSGetConstantBuffers(0, _countof(prevPSCB), prevPSCB);
    m_context->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);
    m_context->IAGetPrimitiveTopology(&prevTopo);
    m_context->IAGetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);

    // --- Ensure VB capacity and upload vertices (unchanged) ---
    EnsureVertexBufferCapacity(m_lineVerts.size());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        memcpy(mapped.pData, m_lineVerts.data(), m_lineVerts.size() * sizeof(DebugLineVertex));
        m_context->Unmap(m_vertexBuffer.Get(), 0);
    }

    // --- Set pipeline for debug drawing ---
    UINT stride = sizeof(DebugLineVertex);
    UINT offset = 0;
    ID3D11Buffer* vb = m_vertexBuffer.Get();
    m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Update and bind viewProj constant buffer (we use slot 0 here — if your renderer uses 0 for world/view/proj, consider moving to a different slot as well)
    struct CBViewProj { DirectX::XMMATRIX viewProj; } cb;
    cb.viewProj = DirectX::XMMatrixTranspose(view * proj);
    m_context->UpdateSubresource(m_cbViewProj.Get(), 0, nullptr, &cb, 0, 0);
    ID3D11Buffer* cbsArr[] = { m_cbViewProj.Get() };
    m_context->VSSetConstantBuffers(0, 1, cbsArr);

    m_context->VSSetShader(m_vs.Get(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, cbsArr); // safe redundant
    m_context->PSSetShader(m_ps.Get(), nullptr, 0);

    // alpha blend on for lines
    float blendFactor[4] = { 0,0,0,0 };
    m_context->OMSetBlendState(m_blendAlpha.Get(), blendFactor, 0xFFFFFFFF);

    // Draw
    m_context->Draw(static_cast<UINT>(m_lineVerts.size()), 0);

    // --- Restore previous pipeline state ---
    m_context->IASetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_context->IASetPrimitiveTopology(prevTopo);
    m_context->IASetInputLayout(prevIL);
    m_context->VSSetShader(prevVS, nullptr, 0);
    m_context->PSSetShader(prevPS, nullptr, 0);
    m_context->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);

    // Restore constant buffers (slot 0..N as we saved)
    m_context->VSSetConstantBuffers(0, _countof(prevVSCB), prevVSCB);
    m_context->PSSetConstantBuffers(0, _countof(prevPSCB), prevPSCB);

    // Release references we acquired via Get*
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
    if (prevIL) prevIL->Release();
    for (auto& p : prevVSCB) if (p) p->Release();
    for (auto& p : prevPSCB) if (p) p->Release();
    if (prevBlend) prevBlend->Release();
    for (auto& p : prevVBs) if (p) p->Release();

    // Next frame: clear the line list
    Clear();
}


void DebugRenderer::Clear()
{
    m_lineVerts.clear();
}
