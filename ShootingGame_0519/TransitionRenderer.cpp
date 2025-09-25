#include "TransitionRenderer.h"
#include "renderer.h" // Renderer::GetDevice()/GetDeviceContext()
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <cassert>
#include <vector>
using Microsoft::WRL::ComPtr;

// シェーダオブジェクト／定数バッファ
static ComPtr<ID3D11VertexShader> s_vsFull;
static ComPtr<ID3D11PixelShader>  s_psFade;
static ComPtr<ID3D11PixelShader>  s_psIris;
static ComPtr<ID3D11Buffer>       s_cbParams;

// CB データ構造
struct CBParams
{
    float color[4];   // for fade color
    float irisCenter[2];
    float irisRadius;
    float padding;
};

// simple vertex shader (SV_VertexID fullscreen triangle)
static const char* g_vs_src = R"(
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };
VSOut VSMain(uint vid : SV_VertexID)
{
    VSOut o;
    // full-screen triangle (3 verts)
    float2 pos[3] = { float2(-1.0f, -1.0f), float2(-1.0f, 3.0f), float2(3.0f, -1.0f) };
    o.pos = float4(pos[vid], 0.0f, 1.0f);
    o.uv = (o.pos.xy * 0.5f) + 0.5f;
    return o;
}
)";

// pixel shader for solid color fade
static const char* g_ps_fade = R"(
cbuffer CB : register(b0)
{
    float4 color;
};
float4 PSMain(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    // draw solid color with alpha
    return color;
}
)";

// pixel shader for iris (center in UV, radius in screen-space fraction)
static const char* g_ps_iris = R"(
cbuffer CB : register(b0)
{
    float4 color;      // fade color (black usually)
    float2 center;     // 0..1 (uv)
    float radius;      // 0..sqrt(2)
    float pad;
};
float4 PSMain(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    float2 d = uv - center;
    float dist = length(d);
    // inside radius => transparent (return float4(0,0,0,0)), outside => color
    float t = smoothstep(radius, radius * 1.02, dist); // soft edge
    return lerp(float4(0,0,0,0), color, t);
}
)";

static bool CompileShader(const char* src, const char* entry, const char* target, ComPtr<ID3DBlob>& blobOut, std::string& errOut)
{
    UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> err;
    HRESULT hr = D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, entry, target, flags, 0, blobOut.GetAddressOf(), err.GetAddressOf());
    if (FAILED(hr))
    {
        if (err) { errOut.assign((const char*)err->GetBufferPointer(), err->GetBufferSize()); }
        else errOut = "Unknown compile error";
        return false;
    }
    return true;
}

bool TransitionRenderer::Init()
{
    ID3D11Device* dev = Renderer::GetDevice();
    if (!dev) return false;

    std::string err;
    ComPtr<ID3DBlob> vsBlob;
    if (!CompileShader(g_vs_src, "VSMain", "vs_5_0", vsBlob, err))
    {
        OutputDebugStringA(("Transition VS compile error: " + err + "\n").c_str());
        return false;
    }
    dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, s_vsFull.GetAddressOf());
    // PS fade
    ComPtr<ID3DBlob> psBlob;
    if (!CompileShader(g_ps_fade, "PSMain", "ps_5_0", psBlob, err))
    {
        OutputDebugStringA(("Transition PS(fade) compile error: " + err + "\n").c_str());
        return false;
    }
    dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, s_psFade.GetAddressOf());

    // PS iris
    if (!CompileShader(g_ps_iris, "PSMain", "ps_5_0", psBlob, err))
    {
        OutputDebugStringA(("Transition PS(iris) compile error: " + err + "\n").c_str());
        return false;
    }
    dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, s_psIris.GetAddressOf());

    // constant buffer
    D3D11_BUFFER_DESC cbd{};
    cbd.ByteWidth = sizeof(CBParams);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dev->CreateBuffer(&cbd, nullptr, s_cbParams.GetAddressOf());

    return true;
}

void TransitionRenderer::Shutdown()
{
    s_vsFull.Reset();
    s_psFade.Reset();
    s_psIris.Reset();
    s_cbParams.Reset();
}

static void setCB(const CBParams& p)
{
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx) return;
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = ctx->Map(s_cbParams.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr) && mapped.pData)
    {
        memcpy(mapped.pData, &p, sizeof(p));
        ctx->Unmap(s_cbParams.Get(), 0);
    }
    ctx->PSSetConstantBuffers(0, 1, s_cbParams.GetAddressOf());
    ctx->VSSetConstantBuffers(0, 1, s_cbParams.GetAddressOf()); // harmless
}

static void saveAndSetCommonState(ID3D11DeviceContext** out_prevVS, ID3D11DeviceContext** out_prevPS, ID3D11VertexShader** out_vs, ID3D11PixelShader** out_ps)
{
    // We'll save VS/PS and some states minimally
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx) return;
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ctx->VSGetShader(&prevVS, nullptr, nullptr);
    ctx->PSGetShader(&prevPS, nullptr, nullptr);

    if (out_prevVS) *out_prevVS = (ID3D11DeviceContext*)prevVS; // storing pointer for later Release cast - we keep convention below
    if (out_prevPS) *out_prevPS = (ID3D11DeviceContext*)prevPS;

    // set our shaders
    ctx->VSSetShader(s_vsFull.Get(), nullptr, 0);
    // PS set done by caller (fade/iris)
}

static void restoreCommonState(ID3D11VertexShader* prevVS, ID3D11PixelShader* prevPS)
{
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx) return;
    ctx->VSSetShader(prevVS, nullptr, 0);
    ctx->PSSetShader(prevPS, nullptr, 0);
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
}

void TransitionRenderer::DrawFullScreen(const float color[4])
{
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx) return;

    // save minimal previous shaders
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ctx->VSGetShader(&prevVS, nullptr, nullptr);
    ctx->PSGetShader(&prevPS, nullptr, nullptr);

    // set our shaders
    ctx->VSSetShader(s_vsFull.Get(), nullptr, 0);
    ctx->PSSetShader(s_psFade.Get(), nullptr, 0);

    // set CB
    CBParams cb{};
    cb.color[0] = color[0];
    cb.color[1] = color[1];
    cb.color[2] = color[2];
    cb.color[3] = color[3];
    setCB(cb);

    // Draw full-screen triangle (3 verts)
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->Draw(3, 0);

    // restore
    ctx->VSSetShader(prevVS, nullptr, 0);
    ctx->PSSetShader(prevPS, nullptr, 0);
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
}

void TransitionRenderer::DrawFullScreenIris(float centerX, float centerY, float radius, const float color[4])
{
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx) return;

    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ctx->VSGetShader(&prevVS, nullptr, nullptr);
    ctx->PSGetShader(&prevPS, nullptr, nullptr);

    ctx->VSSetShader(s_vsFull.Get(), nullptr, 0);
    ctx->PSSetShader(s_psIris.Get(), nullptr, 0);

    CBParams cb{};
    cb.color[0] = color[0];
    cb.color[1] = color[1];
    cb.color[2] = color[2];
    cb.color[3] = color[3];
    cb.irisCenter[0] = centerX;
    cb.irisCenter[1] = centerY;
    cb.irisRadius = radius;
    setCB(cb);

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->Draw(3, 0);

    ctx->VSSetShader(prevVS, nullptr, 0);
    ctx->PSSetShader(prevPS, nullptr, 0);
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
}

void TransitionRenderer::DrawFade(float progress)
{
    // progress 0..1 -> alpha = 0..1 fade out then fade in like "前半暗くなる, 後半明るくなる"
    float alpha;
    if (progress < 0.5f) alpha = progress * 2.0f;
    else alpha = 1.0f - (progress - 0.5f) * 2.0f;
    float color[4] = { 0.0f, 0.0f, 0.0f, alpha }; // 黒フェード
    // Ensure blending: set blend state to alpha blend
    Renderer::SetBlendState(BS_ALPHABLEND);
    Renderer::SetDepthEnable(false);
    DrawFullScreen(color);
    // restore depth / blend? caller (TransitionManager::Draw) doesn't require extra restore since DrawFullScreen restores shaders only.
    Renderer::SetDepthEnable(true);
}

void TransitionRenderer::DrawIris(float progress)
{
    // iris: progress 0 -> closed (black), progress 1 -> open (transparent)
    // We'll compute radius in UV space: 0 (tiny) -> sqrt(2) (cover diag)
    float t = progress;
    // invert so front-half closes? choose simple mapping: radius = (1 - progress) * maxRadius
    float maxRadius = 1.41421356f; // sqrt(2)
    float radius = (1.0f - t) * maxRadius;
    // center at screen center in UV (0.5,0.5)
    float centerX = 0.5f;
    float centerY = 0.5f;
    float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // outside area is black
    Renderer::SetBlendState(BS_ALPHABLEND);
    Renderer::SetDepthEnable(false);
    DrawFullScreenIris(centerX, centerY, radius, color);
    Renderer::SetDepthEnable(true);
}

