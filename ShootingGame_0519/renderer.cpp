/**
 * @file Renderer.cpp
 * @brief DirectX を用いたレンダリング処理を実装する Renderer クラスの定義（実装部）。
 *
 * このファイルでは、Direct3D デバイス、スワップチェーン、レンダーターゲット、定数バッファなどの
 * 初期化と解放、描画の開始・終了処理、各種レンダリング状態（深度、ブレンド、マトリックスなど）の設定を行います。
 */

#include <stdexcept>
#include <d3dcompiler.h>
#include <iostream>
#include <algorithm>
#include "renderer.h"
#include "Application.h"


//------------------------------------------------------------------------------
// スタティックメンバ変数の初期化
//------------------------------------------------------------------------------

D3D_FEATURE_LEVEL       Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ComPtr<ID3D11Device> Renderer::m_Device;
ComPtr<ID3D11DeviceContext> Renderer::m_DeviceContext;
ComPtr<IDXGISwapChain> Renderer::m_SwapChain;
ComPtr<ID3D11RenderTargetView> Renderer::m_RenderTargetView;
ComPtr<ID3D11DepthStencilView> Renderer::m_DepthStencilView;

ComPtr<ID3D11Buffer> Renderer::m_WorldBuffer;
ComPtr<ID3D11Buffer> Renderer::m_ViewBuffer;
ComPtr<ID3D11Buffer> Renderer::m_ProjectionBuffer;
ComPtr<ID3D11Buffer> Renderer::m_MaterialBuffer;
ComPtr<ID3D11Buffer> Renderer::m_LightBuffer;

ComPtr<ID3D11DepthStencilState> Renderer::m_DepthStateEnable;
ComPtr<ID3D11DepthStencilState> Renderer::m_DepthStateDisable;

ComPtr<ID3D11BlendState> Renderer::m_BlendState[MAX_BLENDSTATE];
ComPtr<ID3D11BlendState> Renderer::m_BlendStateATC;

ComPtr<ID3D11VertexShader> Renderer::m_VertexShader;
ComPtr<ID3D11PixelShader>  Renderer::m_PixelShader;
ComPtr<ID3D11InputLayout>  Renderer::m_InputLayout;
ComPtr<ID3D11InputLayout>  Renderer::m_AxisInputLayout;

ComPtr<ID3D11VertexShader> Renderer::m_GridVertexShader;
ComPtr<ID3D11PixelShader>  Renderer::m_GridPixelShader;

//テクスチャ描画用のシェーダーとレイアウト
ComPtr<ID3D11VertexShader> Renderer::m_TextureVertexShader;
ComPtr<ID3D11PixelShader>  Renderer::m_TexturePixelShader;
ComPtr<ID3D11InputLayout>  Renderer::m_TextureInputLayout;

ComPtr<ID3D11DeviceContext> Renderer::m_pContext; // 初期化済みのデバイスコンテキスト
ComPtr<ID3D11BlendState> Renderer::m_pBlendState; // アルファブレンド用
ComPtr<ID3D11Buffer> Renderer::m_pVertexBuffer; // フルスクリーン用頂点バッファ

D3D11_VIEWPORT Renderer::m_Viewport;                // シーン描画用ビューポート
ComPtr<ID3D11RasterizerState> Renderer::m_RasterizerState;

PostProcessSettings Renderer::s_PostProcess{};

ComPtr<ID3D11Texture2D>        Renderer::m_SceneColorTex;
ComPtr<ID3D11RenderTargetView> Renderer::m_SceneColorRTV;
ComPtr<ID3D11ShaderResourceView> Renderer::m_SceneColorSRV;

ComPtr<ID3D11Texture2D>        Renderer::m_PrevSceneColorTex;
ComPtr<ID3D11ShaderResourceView> Renderer::m_PrevSceneColorSRV;

ComPtr<ID3D11PixelShader>      Renderer::m_MotionBlurPixelShader;
ComPtr<ID3D11Buffer>           Renderer::m_PostProcessBuffer;


struct MotionBlurParams
{
    float motionBlurAmount;
    float padding[3]; // 16バイト境界に揃えるためのダミー
};

void Renderer::Init()
{

    HRESULT hr = S_OK;

    //スワップチェインを作成する
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};

    //------------------------------スワップチェインの構造体の中に色々設定------------------------------
    swapChainDesc.BufferCount = 1;//countは1なのでダブルバッファです
    swapChainDesc.BufferDesc.Width = Application::GetWidth();
    swapChainDesc.BufferDesc.Height = Application::GetHeight();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //1ピクセルを構成するビット
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;    //更新頻度
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = Application::GetWindow();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    //-----------------Direct3D デバイス・スワップチェーン・デバイスコンテキストをを作成する-----------------
    hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
        m_SwapChain.GetAddressOf(),
        m_Device.GetAddressOf(),
        &m_FeatureLevel,
        m_DeviceContext.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA("DirectXの初期化に失敗しました。\n");
    }

    //-----------------Direct3D デバイス・スワップチェーン・デバイスコンテキストをを作成する-----------------
    ComPtr<ID3D11Texture2D> renderTarget;
    hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(renderTarget.GetAddressOf()));
    if (SUCCEEDED(hr) && renderTarget) {
        m_Device->CreateRenderTargetView(renderTarget.Get(), nullptr, m_RenderTargetView.GetAddressOf());
    }
    else 
    {
        throw std::runtime_error("Failed to retrieve render target buffer.");
    }

    //----------------------------深度ステンシルバッファ---------------------------
    ComPtr<ID3D11Texture2D> depthStencil;
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = swapChainDesc.BufferDesc.Width;
    textureDesc.Height = swapChainDesc.BufferDesc.Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_D16_UNORM;
    textureDesc.SampleDesc = swapChainDesc.SampleDesc;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = m_Device->CreateTexture2D(&textureDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create depthStencil.");
    }

    //----------------------------深度テクスチャ→深度ステンシルビューとして登録する---------------------------
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = textureDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = m_Device->CreateDepthStencilView(depthStencil.Get(),                   //元となる深度テクスチャ
                                          &depthStencilViewDesc,                //上で作ったビュー設定
                                          m_DepthStencilView.GetAddressOf());   //生成したビューを格納する場所
    if (FAILED(hr)) 
    {
        throw std::runtime_error("Failed to create depthStencilView.");
    }

    m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

    //-----------------------------
    // シーン描画用テクスチャ作成
    //-----------------------------

    D3D11_TEXTURE2D_DESC sceneDesc{};
    sceneDesc.Width = swapChainDesc.BufferDesc.Width;
    sceneDesc.Height = swapChainDesc.BufferDesc.Height;
    sceneDesc.MipLevels = 1;
    sceneDesc.ArraySize = 1;
    sceneDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sceneDesc.SampleDesc = swapChainDesc.SampleDesc;
    sceneDesc.Usage = D3D11_USAGE_DEFAULT;
    sceneDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    hr = m_Device->CreateTexture2D(&sceneDesc, nullptr, m_SceneColorTex.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create scene color texture"); }

    hr = m_Device->CreateRenderTargetView(m_SceneColorTex.Get(), nullptr, m_SceneColorRTV.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create scene RTV"); }

    hr = m_Device->CreateShaderResourceView(m_SceneColorTex.Get(), nullptr, m_SceneColorSRV.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create scene SRV"); }

    // =========================
    // ★ 前フレーム用テクスチャ作成
    // =========================
    D3D11_TEXTURE2D_DESC prevDesc = sceneDesc;
    prevDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 書き込みは CopyResource で行うので RTV は不要

    hr = m_Device->CreateTexture2D(&prevDesc, nullptr, m_PrevSceneColorTex.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create prev scene texture"); }

    hr = m_Device->CreateShaderResourceView(m_PrevSceneColorTex.Get(), nullptr, m_PrevSceneColorSRV.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create prev scene SRV"); }

    // =========================
    // ★ ポストプロセス用定数バッファ
    // =========================
    D3D11_BUFFER_DESC ppDesc{};
    ppDesc.ByteWidth = sizeof(MotionBlurParams);
    ppDesc.Usage = D3D11_USAGE_DEFAULT;
    ppDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ppDesc.CPUAccessFlags = 0;
    ppDesc.MiscFlags = 0;
    ppDesc.StructureByteStride = 0;

    hr = m_Device->CreateBuffer(&ppDesc, nullptr, m_PostProcessBuffer.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create postprocess constant buffer"); }

    // =========================
    // ★ モーションブラー用ピクセルシェーダのコンパイル
    // =========================
    {
        ComPtr<ID3DBlob> blurPsBlob;
        ComPtr<ID3DBlob> blurErr;

        hr = D3DCompileFromFile(
            L"MotionBlurPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0,
            blurPsBlob.GetAddressOf(), blurErr.GetAddressOf());

        if (FAILED(hr))
        {
            if (blurErr) OutputDebugStringA((char*)blurErr->GetBufferPointer());
            throw std::runtime_error("MotionBlur pixel shader compile failed");
        }

        hr = m_Device->CreatePixelShader(
            blurPsBlob->GetBufferPointer(), blurPsBlob->GetBufferSize(),
            nullptr, m_MotionBlurPixelShader.GetAddressOf());

        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create MotionBlur pixel shader");
        }
    }

    m_Viewport.Width = static_cast<FLOAT>(Application::GetWidth());
    m_Viewport.Height = static_cast<FLOAT>(Application::GetHeight());
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_DeviceContext->RSSetViewports(1, &m_Viewport);


    // --- ラスタライザステート設定 ---
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;

    m_Device->CreateRasterizerState(&rasterizerDesc, m_RasterizerState.GetAddressOf());
    // 設定（最初のフレーム）
    m_DeviceContext->RSSetState(m_RasterizerState.Get());

    //-----------------------ブレンドステートの生成-----------------------
    D3D11_BLEND_DESC BlendDesc{};
    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = TRUE;
    BlendDesc.RenderTarget[0].BlendEnable = FALSE;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    m_Device->CreateBlendState(&BlendDesc, m_BlendState[0].GetAddressOf());
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    m_Device->CreateBlendState(&BlendDesc, m_BlendState[1].GetAddressOf());
    m_Device->CreateBlendState(&BlendDesc, m_BlendStateATC.GetAddressOf());

    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    m_Device->CreateBlendState(&BlendDesc, m_BlendState[2].GetAddressOf());

    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    m_Device->CreateBlendState(&BlendDesc, m_BlendState[3].GetAddressOf());

    SetBlendState(BS_ALPHABLEND);

    //-----------------------深度ステンシルステートの設定-----------------------
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    m_Device->CreateDepthStencilState(&depthStencilDesc, m_DepthStateEnable.GetAddressOf());

    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    m_Device->CreateDepthStencilState(&depthStencilDesc, m_DepthStateDisable.GetAddressOf());

    m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable.Get(), 0);

    //-----------------------サンプラーステート設定-----------------------
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy = 4;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11SamplerState> samplerState;
    m_Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
    m_DeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    //-----------------------定数バッファ生成-----------------------
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Matrix4x4);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(float);

    m_Device->CreateBuffer(&bufferDesc, nullptr, m_WorldBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(0, 1, m_WorldBuffer.GetAddressOf());

    m_Device->CreateBuffer(&bufferDesc, nullptr, m_ViewBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(1, 1, m_ViewBuffer.GetAddressOf());

    m_Device->CreateBuffer(&bufferDesc, nullptr, m_ProjectionBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(2, 1, m_ProjectionBuffer.GetAddressOf());
    
    bufferDesc.ByteWidth = sizeof(MATERIAL);
    m_Device->CreateBuffer(&bufferDesc, nullptr, m_MaterialBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(3, 1, m_MaterialBuffer.GetAddressOf());
    m_DeviceContext->PSSetConstantBuffers(3, 1, m_MaterialBuffer.GetAddressOf());

    bufferDesc.ByteWidth = sizeof(LIGHT);
    m_Device->CreateBuffer(&bufferDesc, nullptr, m_LightBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(4, 1, m_LightBuffer.GetAddressOf());
    m_DeviceContext->PSSetConstantBuffers(4, 1, m_LightBuffer.GetAddressOf());

    //-----------------------シェーダーのコンパイル-----------------------
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errBlob;

    // 頂点シェーダーをコンパイル
    hr = D3DCompileFromFile(
        L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0", D3DCOMPILE_DEBUG, 0,
        vsBlob.GetAddressOf(), errBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errBlob) {
            OutputDebugStringA((char*)errBlob->GetBufferPointer());
        }
        throw std::runtime_error("頂点シェーダーのコンパイルに失敗");
    }
    // ピクセルシェーダーをコンパイル
    hr = D3DCompileFromFile(
        L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0,
        psBlob.GetAddressOf(), errBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errBlob) {
            OutputDebugStringA((char*)errBlob->GetBufferPointer());
        }
        throw std::runtime_error("ピクセルシェーダーのコンパイルに失敗");
    }

    //-----------------------シェーダーオブジェクト作成-----------------------
    m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_VertexShader.GetAddressOf());
    m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_PixelShader.GetAddressOf());

    //-----------------------頂点レイアウトを作成-----------------------
    // BasicVertexShader.hlsl の VS_Input に合わせて
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX_3D, Position),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX_3D, Normal),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX_3D, Diffuse),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,         0, offsetof(VERTEX_3D, TexCoord),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT,    0, offsetof(VERTEX_3D, BoneIndex),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONEWEIGHT",0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX_3D, BoneWeight), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


    m_Device->CreateInputLayout(layoutDesc, _countof(layoutDesc),vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),m_InputLayout.GetAddressOf());

    //-----------------------最後にシェーダー／レイアウトをセット-----------------------
    m_DeviceContext->IASetInputLayout(m_InputLayout.Get());
    m_DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
    m_DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);

    //グリッド用の頂点シェーダーをコンパイル
    hr = D3DCompileFromFile(
        L"GridVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0", D3DCOMPILE_DEBUG, 0,
        vsBlob.GetAddressOf(), errBlob.GetAddressOf());
    if (FAILED(hr))
    {
        if (errBlob) OutputDebugStringA((char*)errBlob->GetBufferPointer());
        throw std::runtime_error("Grid頂点シェーダーのコンパイルに失敗");
    }

    //グリッド用のピクセルシェーダーをコンパイル
    hr = D3DCompileFromFile(
        L"GridPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0,
        psBlob.GetAddressOf(), errBlob.GetAddressOf());
    if (FAILED(hr))
    {
        if (errBlob) OutputDebugStringA((char*)errBlob->GetBufferPointer());
        throw std::runtime_error("Gridピクセルシェーダーのコンパイルに失敗");
    }

    // シェーダーオブジェクト作成
    m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_GridVertexShader.GetAddressOf());
    m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_GridPixelShader.GetAddressOf());

    // --- テクスチャ用シェーダーのコンパイル ---
    ComPtr<ID3DBlob> texVsBlob;
    ComPtr<ID3DBlob> texPsBlob;
    ComPtr<ID3DBlob> texErrBlob;

    // 頂点シェーダーコンパイル
    hr = D3DCompileFromFile(
        L"TextureVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0", D3DCOMPILE_DEBUG, 0,
        texVsBlob.GetAddressOf(), texErrBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (texErrBlob) OutputDebugStringA((char*)texErrBlob->GetBufferPointer());
        throw std::runtime_error("Texture vertex shader compile failed");
    }

    // ピクセルシェーダーコンパイル
    hr = D3DCompileFromFile(
        L"TexturePixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0,
        texPsBlob.GetAddressOf(), texErrBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (texErrBlob) OutputDebugStringA((char*)texErrBlob->GetBufferPointer());
        throw std::runtime_error("Texture pixel shader compile failed");
    }

    // シェーダーオブジェクト作成
    m_Device->CreateVertexShader(texVsBlob->GetBufferPointer(), texVsBlob->GetBufferSize(), nullptr, m_TextureVertexShader.GetAddressOf());
    m_Device->CreatePixelShader(texPsBlob->GetBufferPointer(), texPsBlob->GetBufferSize(), nullptr, m_TexturePixelShader.GetAddressOf());

    // 入力レイアウト（位置(float3) + UV(float2)）
    D3D11_INPUT_ELEMENT_DESC texLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_Device->CreateInputLayout(texLayoutDesc, _countof(texLayoutDesc),
        texVsBlob->GetBufferPointer(), texVsBlob->GetBufferSize(), m_TextureInputLayout.GetAddressOf());


}


//Direct3Dのリソースは明示的に解放しないとメモリリークが発生するため、
//ComPtr::Reset()で安全にリソースを開放しています。
void Renderer::Uninit()
{
    for (auto& bs : m_BlendState)
    {
        bs.Reset();
    }
    m_BlendStateATC.Reset();
    m_DepthStateEnable.Reset();
    m_DepthStateDisable.Reset();
    m_WorldBuffer.Reset();
    m_ViewBuffer.Reset();
    m_ProjectionBuffer.Reset();
    m_LightBuffer.Reset();
    m_MaterialBuffer.Reset();
    m_RenderTargetView.Reset();
    m_SwapChain.Reset();
    m_DeviceContext.Reset();
    m_Device.Reset();

    char buf[256];
    sprintf_s(buf, "Uninit: device=%p context=%p\n", m_Device.Get(), m_pContext.Get());
    OutputDebugStringA(buf);
    m_pContext.Reset();
    m_Device.Reset();
}


 //画面を指定色（青色）でクリア
 //深度バッファも初期化
 //毎フレーム必ず呼び出して、前のフレームの残像を消します。
 
void Renderer::Begin()
{
    ID3D11RenderTargetView* rtv = m_SceneColorRTV.Get();
    m_DeviceContext->OMSetRenderTargets(1, &rtv, m_DepthStencilView.Get());

    float clearColor[4] = { 0, 0, 0, 1 };
    m_DeviceContext->ClearRenderTargetView(m_SceneColorRTV.Get(), clearColor);
    m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0);
}

void Renderer::End()
{
    ApplyMotionBlur();
    m_SwapChain->Present(1, 0);
}

void Renderer::Present()
{
    m_SwapChain->Present(1, 0);
}

void Renderer::SetDepthEnable(bool Enable)
{
    m_DeviceContext->OMSetDepthStencilState(
        Enable ? m_DepthStateEnable.Get() : m_DepthStateDisable.Get(), 0);
}

/**
 * @brief Alpha To Coverage（半透明表現用）のON/OFFを切り替えます。
 * @param Enable trueならATC有効、falseなら無効
 * @details
 * マルチサンプリング＋アルファブレンドの高度な合成を行う機能です。
 */
void Renderer::SetATCEnable(bool Enable)
{
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_DeviceContext->OMSetBlendState(
        Enable ? m_BlendStateATC.Get() : m_BlendState[0].Get(),
        blendFactor, 0xffffffff);
}

/**
 * @brief 2D描画用に、単純なワールド・ビュー・プロジェクション行列をセットします。
 *
 * @details
 * 画面左上を原点とする2D直交投影行列を生成し、各行列バッファに設定します。
 */
void Renderer::SetWorldViewProjection2D()
{
    Matrix4x4 world = Matrix4x4::Identity.Transpose();
    m_DeviceContext->UpdateSubresource(m_WorldBuffer.Get(), 0, nullptr, &world, 0, 0);

    Matrix4x4 view = Matrix4x4::Identity.Transpose();
    m_DeviceContext->UpdateSubresource(m_ViewBuffer.Get(), 0, nullptr, &view, 0, 0);

    Matrix4x4 projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f,
        static_cast<float>(Application::GetWidth()),
        static_cast<float>(Application::GetHeight()),
        0.0f,
        0.0f,
        1.0f);
    projection = projection.Transpose();
    m_DeviceContext->UpdateSubresource(m_ProjectionBuffer.Get(), 0, nullptr, &projection, 0, 0);
}

/**
 * @brief 任意のワールド行列をシェーダーにセットします。
 * @param WorldMatrix ワールド行列へのポインタ
 */
void Renderer::SetWorldMatrix(Matrix4x4* WorldMatrix)
{
    Matrix4x4 mat = WorldMatrix->Transpose();
    m_DeviceContext->UpdateSubresource(m_WorldBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief 任意のビュー行列をシェーダーにセットします。
 * @param ViewMatrix ビュー行列へのポインタ
 */
void Renderer::SetViewMatrix(SimpleMath::Matrix ViewMatrix)
{
    SimpleMath::Matrix mat = ViewMatrix.Transpose();
    m_DeviceContext->UpdateSubresource(m_ViewBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief 任意のプロジェクション行列をシェーダーにセットします。
 * @param ProjectionMatrix 射影行列へのポインタ
 */
void Renderer::SetProjectionMatrix(SimpleMath::Matrix ProjectionMatrix)
{
    SimpleMath::Matrix mat = ProjectionMatrix.Transpose();
    m_DeviceContext->UpdateSubresource(m_ProjectionBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief マテリアル（表面材質）情報をセットします。
 * @param Material マテリアル情報
 */
void Renderer::SetMaterial(MATERIAL Material)
{
    m_DeviceContext->UpdateSubresource(m_MaterialBuffer.Get(), 0, nullptr, &Material, 0, 0);
}

/**
 * @brief ライト（光源）情報をセットします。
 * @param Light ライト情報
 */
void Renderer::SetLight(LIGHT Light)
{
    m_DeviceContext->UpdateSubresource(m_LightBuffer.Get(), 0, nullptr, &Light, 0, 0);
}

/**
 * @brief 指定したブレンドステートをセットします。
 * @param nBlendState 使用するブレンドステートの種類
 */
void Renderer::SetBlendState(int nBlendState)
{
    if (nBlendState >= 0 && nBlendState < MAX_BLENDSTATE) {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_DeviceContext->OMSetBlendState(m_BlendState[nBlendState].Get(), blendFactor, 0xffffffff);
    }
}

/**
 * @brief 面の除外（カリング）を無効または有効にします。
 * @param cullflag trueでカリングON（通常）、falseでカリングOFF（両面描画）
 */
void Renderer::DisableCulling(bool cullflag)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = cullflag ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    ComPtr<ID3D11RasterizerState> pRasterizerState;
    HRESULT hr = m_Device->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf());
    if (FAILED(hr))
        return;

    m_DeviceContext->RSSetState(pRasterizerState.Get());
}


/**
 * @brief ラスタライザステートのフィルモード（塗りつぶし/ワイヤーフレーム）を設定します。
 * @param FillMode D3D11_FILL_SOLID または D3D11_FILL_WIREFRAME
 */
void Renderer::SetFillMode(D3D11_FILL_MODE FillMode)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = FillMode;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;

    ComPtr<ID3D11RasterizerState> rs;
    m_Device->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    m_DeviceContext->RSSetState(rs.Get());
}

/**
 * @brief 深度テストを常にパスさせる設定に変更します。
 *
 * @details
 * - 深度テストは有効（DepthEnable = TRUE）
 * - ただし、常に「描画OK」（DepthFunc = D3D11_COMPARISON_ALWAYS）
 * - 深度バッファにも書き込む（DepthWriteMask = ALL）
 */
void Renderer::SetDepthAllwaysWrite()
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS; // 常に深度テスト成功
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.StencilEnable = FALSE; // ステンシルテストは無効

    ComPtr<ID3D11DepthStencilState> pDepthStencilState;
    HRESULT hr = m_Device->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        m_DeviceContext->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
    }
}

void Renderer::SetPostProcessSettings(const PostProcessSettings& settings)
{
    s_PostProcess = settings;
}

const PostProcessSettings& Renderer::GetPostProcessSettings()
{
    return s_PostProcess;
}

/**
 * @brief テクスチャを画面上の指定位置・サイズに描画します。
 * @param texture 描画対象のテクスチャ（ShaderResourceView）
 * @param position 画面上の左上位置（ピクセル座標）
 * @param size 描画するサイズ（幅と高さのピクセル単位）
 */
void Renderer::DrawTexture(ID3D11ShaderResourceView* texture, const Vector2& position, const Vector2& size)
{
    //std::cout << "[Renderer] DrawTexture start texture=" << texture << " pos=(" << position.x << "," << position.y << ") size=(" << size.x << "," << size.y << ")\n";
       
    if (!texture) { OutputDebugStringA("DBG: DrawTexture - texture null\n"); return; }

    // -------- Save GPU state we'll change --------
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ID3D11InputLayout* prevIL = nullptr;
    ID3D11Buffer* prevVSCB[1] = { nullptr };
    ID3D11Buffer* prevPSCB[1] = { nullptr };
    ID3D11BlendState* prevBlend = nullptr;
    FLOAT prevBlendFactor[4] = { 0,0,0,0 };
    UINT prevSampleMask = 0xFFFFFFFF;
    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencilRef = 0;
    ID3D11RasterizerState* prevRS = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* prevVBs[1] = { nullptr };
    UINT prevStrides[1] = { 0 };
    UINT prevOffsets[1] = { 0 };
    ID3D11ShaderResourceView* prevPSRV[1] = { nullptr };
    ID3D11SamplerState* prevSampler[1] = { nullptr };

    m_DeviceContext->VSGetShader(&prevVS, nullptr, nullptr);
    m_DeviceContext->PSGetShader(&prevPS, nullptr, nullptr);
    m_DeviceContext->IAGetInputLayout(&prevIL);
    m_DeviceContext->VSGetConstantBuffers(0, 1, prevVSCB);
    m_DeviceContext->PSGetConstantBuffers(0, 1, prevPSCB);
    m_DeviceContext->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);
    m_DeviceContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);
    m_DeviceContext->RSGetState(&prevRS);
    m_DeviceContext->IAGetPrimitiveTopology(&prevTopo);
    m_DeviceContext->IAGetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_DeviceContext->PSGetShaderResources(0, 1, prevPSRV);
    m_DeviceContext->PSGetSamplers(0, 1, prevSampler);

    // -------- Build vertices (left-top origin) --------
    struct Vertex { Vector3 pos; Vector2 uv; };
    float x = position.x;
    float y = position.y;
    float w = size.x;
    float h = size.y;
    Vertex vertices[6] =
    {
        {{x,     y,     0.0f}, {0.0f, 0.0f}},
        {{x + w, y,     0.0f}, {1.0f, 0.0f}},
        {{x,     y + h, 0.0f}, {0.0f, 1.0f}},

        {{x + w, y,     0.0f}, {1.0f, 0.0f}},
        {{x + w, y + h, 0.0f}, {1.0f, 1.0f}},
        {{x,     y + h, 0.0f}, {0.0f, 1.0f}},
    };

    // create ephemeral VB (ComPtr to auto-release)
    ComPtr<ID3D11Buffer> vb;
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = UINT(sizeof(vertices));
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;
    HRESULT hr = m_Device->CreateBuffer(&vbDesc, &initData, vb.GetAddressOf());
    if (FAILED(hr))
    {
        // failed to create VB -> restore and return
        // restore below (we'll restore minimal pieces)
        if (prevVS) prevVS->Release();
        if (prevPS) prevPS->Release();
        if (prevIL) prevIL->Release();
        if (prevVSCB[0]) prevVSCB[0]->Release();
        if (prevPSCB[0]) prevPSCB[0]->Release();
        if (prevBlend) prevBlend->Release();
        if (prevDSS) prevDSS->Release();
        if (prevRS) prevRS->Release();
        if (prevVBs[0]) prevVBs[0]->Release();
        if (prevPSRV[0]) prevPSRV[0]->Release();
        if (prevSampler[0]) prevSampler[0]->Release();
        return;
    }

    // -------- Bind 2D shaders / input layout and vertices --------
    m_DeviceContext->IASetInputLayout(m_TextureInputLayout.Get());
    m_DeviceContext->VSSetShader(m_TextureVertexShader.Get(), nullptr, 0);
    m_DeviceContext->PSSetShader(m_TexturePixelShader.Get(), nullptr, 0);
    SetWorldViewProjection2D(); // your helper to set matrices to CBs

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vbPtr = vb.Get();
    m_DeviceContext->IASetVertexBuffers(0, 1, &vbPtr, &stride, &offset);
    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Bind texture + optional sampler (sampler slot 0)
    m_DeviceContext->PSSetShaderResources(0, 1, &texture);
    // if you have a sampler object: m_DeviceContext->PSSetSamplers(0,1,m_Sampler.GetAddressOf());

    // Draw
    m_DeviceContext->Draw(6, 0);

    // -------- Unbind our SRV to avoid binding it beyond this call --------
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_DeviceContext->PSSetShaderResources(0, 1, nullSRV);

    // -------- Restore previously saved GPU state --------
    // restore samplers
    m_DeviceContext->PSSetSamplers(0, 1, prevSampler);

    // restore SRV
    m_DeviceContext->PSSetShaderResources(0, 1, prevPSRV);

    // restore VB
    m_DeviceContext->IASetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);

    // restore primitive topology
    m_DeviceContext->IASetPrimitiveTopology(prevTopo);

    // restore rasterizer
    m_DeviceContext->RSSetState(prevRS);

    // restore depth-stencil
    m_DeviceContext->OMSetDepthStencilState(prevDSS, prevStencilRef);

    // restore blend
    m_DeviceContext->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);

    // restore constant buffers
    m_DeviceContext->VSSetConstantBuffers(0, 1, prevVSCB);
    m_DeviceContext->PSSetConstantBuffers(0, 1, prevPSCB);

    // restore shaders & input layout
    m_DeviceContext->VSSetShader(prevVS, nullptr, 0);
    m_DeviceContext->PSSetShader(prevPS, nullptr, 0);
    m_DeviceContext->IASetInputLayout(prevIL);

    // Release the references we acquired via Get*
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
    if (prevIL) prevIL->Release();
    for (auto p : prevVSCB) if (p) p->Release();
    for (auto p : prevPSCB) if (p) p->Release();
    if (prevBlend) prevBlend->Release();
    if (prevDSS) prevDSS->Release();
    if (prevRS) prevRS->Release();
    for (auto p : prevVBs) if (p) p->Release();
    for (auto p : prevPSRV) if (p) p->Release();
    for (auto p : prevSampler) if (p) p->Release();

    //std::cout << "[Renderer] DrawTexture end\n";
}

void Renderer::ApplyMotionBlur()
{
    if (!m_SceneColorTex || !m_SceneColorSRV || !m_PrevSceneColorSRV)
    {
        return;
    }

    // ブラー量を 0～1 に制限
    float blur = std::clamp(s_PostProcess.motionBlurAmount, 0.0f, 1.0f);

    // BackBuffer のリソース取得（CopyResource に使う）
    ID3D11Resource* backRes = nullptr;
    m_RenderTargetView->GetResource(&backRes);

    // ブラー量がほぼゼロ → シーン画像をコピーするだけ
    if (blur <= 0.001f)
    {
        if (backRes)
        {
            // バックバッファにシーン画像をコピー
            m_DeviceContext->CopyResource(backRes, m_SceneColorTex.Get());

            // 次フレーム用に prev にもコピー
            m_DeviceContext->CopyResource(m_PrevSceneColorTex.Get(), m_SceneColorTex.Get());

            backRes->Release();
        }
        return;
    }

    // ================================
    //  GPU ステート保存
    // ================================
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ID3D11InputLayout* prevIL = nullptr;
    ID3D11Buffer* prevVSCB[1] = { nullptr };
    ID3D11Buffer* prevPSCB[1] = { nullptr };
    ID3D11BlendState* prevBlend = nullptr;
    FLOAT               prevBlendFactor[4] = { 0,0,0,0 };
    UINT                prevSampleMask = 0xFFFFFFFF;
    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencilRef = 0;
    ID3D11RasterizerState* prevRS = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY prevTopo = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* prevVBs[1] = { nullptr };
    UINT prevStrides[1] = { 0 };
    UINT prevOffsets[1] = { 0 };
    ID3D11ShaderResourceView* prevPSRV[2] = { nullptr, nullptr };
    ID3D11SamplerState* prevSampler[1] = { nullptr };

    m_DeviceContext->VSGetShader(&prevVS, nullptr, nullptr);
    m_DeviceContext->PSGetShader(&prevPS, nullptr, nullptr);
    m_DeviceContext->IAGetInputLayout(&prevIL);
    m_DeviceContext->VSGetConstantBuffers(0, 1, prevVSCB);
    m_DeviceContext->PSGetConstantBuffers(0, 1, prevPSCB);
    m_DeviceContext->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);
    m_DeviceContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);
    m_DeviceContext->RSGetState(&prevRS);
    m_DeviceContext->IAGetPrimitiveTopology(&prevTopo);
    m_DeviceContext->IAGetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_DeviceContext->PSGetShaderResources(0, 2, prevPSRV);
    m_DeviceContext->PSGetSamplers(0, 1, prevSampler);

    // ================================
    // バックバッファへ描画する
    // ================================
    ID3D11RenderTargetView* backRTV = m_RenderTargetView.Get();
    m_DeviceContext->OMSetRenderTargets(1, &backRTV, nullptr);

    // ================================
    // フルスクリーンクアッド作成
    // ================================
    struct Vertex { Vector3 pos; Vector2 uv; };
    float w = (float)Application::GetWidth();
    float h = (float)Application::GetHeight();

    Vertex vertices[6] =
    {
        {{0, 0, 0}, {0, 0}},
        {{w, 0, 0}, {1, 0}},
        {{0, h, 0}, {0, 1}},

        {{w, 0, 0}, {1, 0}},
        {{w, h, 0}, {1, 1}},
        {{0, h, 0}, {0, 1}},
    };

    ComPtr<ID3D11Buffer> vb;
    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = vertices;

    HRESULT hr = m_Device->CreateBuffer(&vbDesc, &init, vb.GetAddressOf());
    if (FAILED(hr))
    {
        if (backRes) backRes->Release();
        return;
    }

    // ================================
    // MotionBlur シェーダー設定
    // ================================
    m_DeviceContext->IASetInputLayout(m_TextureInputLayout.Get());
    m_DeviceContext->VSSetShader(m_TextureVertexShader.Get(), nullptr, 0);
    m_DeviceContext->PSSetShader(m_MotionBlurPixelShader.Get(), nullptr, 0);

    SetWorldViewProjection2D();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vbPtr = vb.Get();
    m_DeviceContext->IASetVertexBuffers(0, 1, &vbPtr, &stride, &offset);
    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ================================
    // SRV をセット（SceneTex & PrevTex）
    // ================================
    ID3D11ShaderResourceView* srvs[2] =
    {
        m_SceneColorSRV.Get(),      // t0
        m_PrevSceneColorSRV.Get(),  // t1
    };
    m_DeviceContext->PSSetShaderResources(0, 2, srvs);

    // ================================
    // ★ 定数バッファ更新（最重要ポイント）★
    // ================================
    PostProcessSettings cbData = s_PostProcess;
    cbData.motionBlurAmount = blur;  // クランプ済みに書き換え

    m_DeviceContext->UpdateSubresource(
        m_PostProcessBuffer.Get(), 0, nullptr, &cbData, 0, 0
    );

    ID3D11Buffer* ppCB = m_PostProcessBuffer.Get();
    m_DeviceContext->PSSetConstantBuffers(0, 1, &ppCB);

    // ================================
    // 描画
    // ================================
    m_DeviceContext->Draw(6, 0);

    // SRV 解放
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    m_DeviceContext->PSSetShaderResources(0, 2, nullSRV);

    // ================================
    // GPU のステートをすべて復元
    // ================================
    m_DeviceContext->PSSetSamplers(0, 1, prevSampler);
    m_DeviceContext->PSSetShaderResources(0, 2, prevPSRV);
    m_DeviceContext->IASetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_DeviceContext->IASetPrimitiveTopology(prevTopo);
    m_DeviceContext->RSSetState(prevRS);
    m_DeviceContext->OMSetDepthStencilState(prevDSS, prevStencilRef);
    m_DeviceContext->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);
    m_DeviceContext->VSSetConstantBuffers(0, 1, prevVSCB);
    m_DeviceContext->PSSetConstantBuffers(0, 1, prevPSCB);
    m_DeviceContext->VSSetShader(prevVS, nullptr, 0);
    m_DeviceContext->PSSetShader(prevPS, nullptr, 0);
    m_DeviceContext->IASetInputLayout(prevIL);

    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
    if (prevIL) prevIL->Release();
    if (prevVSCB[0]) prevVSCB[0]->Release();
    if (prevPSCB[0]) prevPSCB[0]->Release();
    if (prevBlend) prevBlend->Release();
    if (prevDSS) prevDSS->Release();
    if (prevRS) prevRS->Release();
    if (prevVBs[0]) prevVBs[0]->Release();
    if (prevSampler[0]) prevSampler[0]->Release();
    for (auto& p : prevPSRV) if (p) p->Release();

    // ================================
    // 最後に「今のフレーム画像」を prev に保存
    // ================================
    if (backRes)
    {
        m_DeviceContext->CopyResource(m_PrevSceneColorTex.Get(), m_SceneColorTex.Get());
        backRes->Release();
    }
}

void Renderer::DrawReticle(ID3D11ShaderResourceView* texture, const POINT& center, const Vector2& size)
{
    if (!texture) return;

    // Save/restore depth & blend state quickly (we'll use DrawTexture which restores most state)
    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencilRef = 0;
    m_DeviceContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);

    ID3D11BlendState* prevBlend = nullptr;
    FLOAT prevBlendFactor[4] = { 0,0,0,0 };
    UINT prevSampleMask = 0xFFFFFFFF;
    m_DeviceContext->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);

    // turn off depth, enable alpha blend (assume you have BS_ALPHABLEND created and stored)
    SetDepthEnable(false);
    SetBlendState(BS_ALPHABLEND);

    Vector2 topLeft;
    topLeft.x = static_cast<float>(center.x) - size.x * 0.5f;
    topLeft.y = static_cast<float>(center.y) - size.y * 0.5f;
    DrawTexture(texture, topLeft, size);

    // restore blend/depth (DrawTexture already restores shaders/IL etc.)
    // but if SetBlendState/SetDepthEnable modified device state outside DrawTexture, restore here:
    m_DeviceContext->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);
    m_DeviceContext->OMSetDepthStencilState(prevDSS, prevStencilRef);

    if (prevDSS) prevDSS->Release();
    if (prevBlend) prevBlend->Release();
}

void Renderer::BeginSceneRenderTarget()
{
    ID3D11RenderTargetView* rtv = m_SceneColorRTV.Get();
    ID3D11DepthStencilView* dsv = m_DepthStencilView.Get();

    // シーン用RTV + 深度
    m_DeviceContext->OMSetRenderTargets(1, &rtv, dsv);

    float clearColor[4] = { 0, 0, 0, 1 };
    m_DeviceContext->ClearRenderTargetView(rtv, clearColor);
    m_DeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // シーン用ステート
    m_DeviceContext->RSSetState(m_RasterizerState.Get());
    m_DeviceContext->RSSetViewports(1, &m_Viewport);
}


void Renderer::BeginBackBuffer()
{
    ID3D11RenderTargetView* rtv = m_RenderTargetView.Get();
    m_DeviceContext->OMSetRenderTargets(1, &rtv, nullptr);

    float clearColor[4] = { 0, 0, 0, 1 };
    m_DeviceContext->ClearRenderTargetView(rtv, clearColor);

    // UI では Z は不要なので深度は無し
    // ラスタライザとビューポートもデフォルトで OK
}





//m_DeviceContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);