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
#include "TransitionManager.h"


//------------------------------------------------------------------------------
// スタティックメンバ変数の初期化
//------------------------------------------------------------------------------

D3D_FEATURE_LEVEL       Renderer::m_featureLevel = D3D_FEATURE_LEVEL_11_0;

ComPtr<ID3D11Device> Renderer::m_device;
ComPtr<ID3D11DeviceContext> Renderer::m_deviceContext;
ComPtr<IDXGISwapChain> Renderer::m_swapChain;
ComPtr<ID3D11RenderTargetView> Renderer::m_renderTargetView;
ComPtr<ID3D11DepthStencilView> Renderer::m_depthStencilView;

ComPtr<ID3D11Buffer> Renderer::m_worldBuffer;
ComPtr<ID3D11Buffer> Renderer::m_viewBuffer;
ComPtr<ID3D11Buffer> Renderer::m_projectionBuffer;
ComPtr<ID3D11Buffer> Renderer::m_materialBuffer;
ComPtr<ID3D11Buffer> Renderer::m_lightBuffer;

ComPtr<ID3D11DepthStencilState> Renderer::m_depthStateEnable;
ComPtr<ID3D11DepthStencilState> Renderer::m_depthStateDisable;

ComPtr<ID3D11BlendState> Renderer::m_blendState[MAX_BLENDSTATE];
ComPtr<ID3D11BlendState> Renderer::m_blendStateATC;

ComPtr<ID3D11VertexShader> Renderer::m_vertexShader;
ComPtr<ID3D11PixelShader>  Renderer::m_pixelShader;
ComPtr<ID3D11InputLayout>  Renderer::m_inputLayout;
ComPtr<ID3D11InputLayout>  Renderer::m_axisInputLayout;

ComPtr<ID3D11VertexShader> Renderer::m_gridVertexShader;
ComPtr<ID3D11PixelShader>  Renderer::m_gridPixelShader;

//テクスチャ描画用のシェーダーとレイアウト
ComPtr<ID3D11VertexShader> Renderer::m_textureVertexShader;
ComPtr<ID3D11PixelShader>  Renderer::m_texturePixelShader;
ComPtr<ID3D11InputLayout>  Renderer::m_textureInputLayout;
ComPtr<ID3D11Buffer>  Renderer::m_textureAlphaBuffer;

ComPtr<ID3D11DeviceContext> Renderer::m_pContext; // 初期化済みのデバイスコンテキスト
ComPtr<ID3D11BlendState> Renderer::m_pBlendState; // アルファブレンド用
ComPtr<ID3D11Buffer> Renderer::m_pVertexBuffer; // フルスクリーン用頂点バッファ

D3D11_VIEWPORT Renderer::m_viewport;                // シーン描画用ビューポート
ComPtr<ID3D11RasterizerState> Renderer::m_rasterizerState;

PostProcessSettings Renderer::s_postProcess{};

ComPtr<ID3D11Texture2D>        Renderer::m_sceneColorTex;
ComPtr<ID3D11RenderTargetView> Renderer::m_sceneColorRTV;
ComPtr<ID3D11ShaderResourceView> Renderer::m_sceneColorSRV;

ComPtr<ID3D11Texture2D>        Renderer::m_prevSceneColorTex;
ComPtr<ID3D11ShaderResourceView> Renderer::m_prevSceneColorSRV;

ComPtr<ID3D11PixelShader>      Renderer::m_motionBlurPixelShader;
ComPtr<ID3D11Buffer>           Renderer::m_postProcessBuffer;


struct MotionBlurParams
{
    float motionBlurAmount;
    float padding[3]; // 16バイト境界に揃えるためのダミー
};

Microsoft::WRL::ComPtr<ID3DBlob> Renderer::CompileShader(
    const wchar_t* filePath,
    const char* entryPoint,
    const char* target)
{
    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG) || defined(DEBUG)
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filePath,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        target,
        compileFlags,
        0,
        shaderBlob.GetAddressOf(),
        errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            // エラーメッセージを出力（デバッグ用）
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }

        // 失敗時は例外を投げる（呼び出し側でキャッチ/ハンドルする想定）
        throw std::runtime_error("Shader compile failed");
    }

    return shaderBlob;
}

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
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &m_featureLevel,
        m_deviceContext.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA("DirectXの初期化に失敗しました。\n");
    }

    //-----------------Direct3D デバイス・スワップチェーン・デバイスコンテキストをを作成する-----------------
    ComPtr<ID3D11Texture2D> renderTarget;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(renderTarget.GetAddressOf()));
    if (SUCCEEDED(hr) && renderTarget) {
        m_device->CreateRenderTargetView(renderTarget.Get(), nullptr, m_renderTargetView.GetAddressOf());
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
    hr = m_device->CreateTexture2D(&textureDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create depthStencil.");
    }

    //----------------------------深度テクスチャ→深度ステンシルビューとして登録する---------------------------
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = textureDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = m_device->CreateDepthStencilView(depthStencil.Get(),                   //元となる深度テクスチャ
                                          &depthStencilViewDesc,                //上で作ったビュー設定
                                          m_depthStencilView.GetAddressOf());   //生成したビューを格納する場所
    if (FAILED(hr)) 
    {
        throw std::runtime_error("Failed to create depthStencilView.");
    }

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

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

    hr = m_device->CreateTexture2D(&sceneDesc, nullptr, m_sceneColorTex.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create scene color texture"); }

    hr = m_device->CreateRenderTargetView(m_sceneColorTex.Get(), nullptr, m_sceneColorRTV.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create scene RTV"); }

    hr = m_device->CreateShaderResourceView(m_sceneColorTex.Get(), nullptr, m_sceneColorSRV.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create scene SRV"); }

    // =========================
    // ★ 前フレーム用テクスチャ作成
    // =========================
    D3D11_TEXTURE2D_DESC prevDesc = sceneDesc;
    prevDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 書き込みは CopyResource で行うので RTV は不要

    hr = m_device->CreateTexture2D(&prevDesc, nullptr, m_prevSceneColorTex.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create prev scene texture"); }

    hr = m_device->CreateShaderResourceView(m_prevSceneColorTex.Get(), nullptr, m_prevSceneColorSRV.GetAddressOf());
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

    hr = m_device->CreateBuffer(&ppDesc, nullptr, m_postProcessBuffer.GetAddressOf());
    if (FAILED(hr)) { throw std::runtime_error("Failed to create postprocess constant buffer"); }

    // =========================
    // ★ モーションブラー用ピクセルシェーダのコンパイル
    // =========================
    auto blurPsBlob = CompileShader( L"MotionBlurPixelShader.hlsl",
                                     "PSMain", "ps_5_0");

     hr = m_device->CreatePixelShader(
        blurPsBlob->GetBufferPointer(),
        blurPsBlob->GetBufferSize(),
        nullptr,
        m_motionBlurPixelShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create MotionBlur pixel shader");
    }

    m_viewport.Width = static_cast<FLOAT>(Application::GetWidth());
    m_viewport.Height = static_cast<FLOAT>(Application::GetHeight());
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_deviceContext->RSSetViewports(1, &m_viewport);


    // --- ラスタライザステート設定 ---
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;

    m_device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
    // 設定（最初のフレーム）
    m_deviceContext->RSSetState(m_rasterizerState.Get());

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

    m_device->CreateBlendState(&BlendDesc, m_blendState[0].GetAddressOf());
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    m_device->CreateBlendState(&BlendDesc, m_blendState[1].GetAddressOf());
    m_device->CreateBlendState(&BlendDesc, m_blendStateATC.GetAddressOf());

    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    m_device->CreateBlendState(&BlendDesc, m_blendState[2].GetAddressOf());

    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    m_device->CreateBlendState(&BlendDesc, m_blendState[3].GetAddressOf());

    SetBlendState(BS_ALPHABLEND);

    //-----------------------深度ステンシルステートの設定-----------------------
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStateEnable.GetAddressOf());

    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStateDisable.GetAddressOf());

    m_deviceContext->OMSetDepthStencilState(m_depthStateEnable.Get(), 0);

    //-----------------------サンプラーステート設定-----------------------
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy = 4;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11SamplerState> samplerState;
    m_device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    //-----------------------定数バッファ生成-----------------------
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Matrix4x4);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(float);

    m_device->CreateBuffer(&bufferDesc, nullptr, m_worldBuffer.GetAddressOf());
    m_deviceContext->VSSetConstantBuffers(0, 1, m_worldBuffer.GetAddressOf());

    m_device->CreateBuffer(&bufferDesc, nullptr, m_viewBuffer.GetAddressOf());
    m_deviceContext->VSSetConstantBuffers(1, 1, m_viewBuffer.GetAddressOf());

    m_device->CreateBuffer(&bufferDesc, nullptr, m_projectionBuffer.GetAddressOf());
    m_deviceContext->VSSetConstantBuffers(2, 1, m_projectionBuffer.GetAddressOf());
    
    bufferDesc.ByteWidth = sizeof(MATERIAL);
    m_device->CreateBuffer(&bufferDesc, nullptr, m_materialBuffer.GetAddressOf());
    m_deviceContext->VSSetConstantBuffers(3, 1, m_materialBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(3, 1, m_materialBuffer.GetAddressOf());

    bufferDesc.ByteWidth = sizeof(LIGHT);
    m_device->CreateBuffer(&bufferDesc, nullptr, m_lightBuffer.GetAddressOf());
    m_deviceContext->VSSetConstantBuffers(4, 1, m_lightBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(4, 1, m_lightBuffer.GetAddressOf());

    D3D11_BUFFER_DESC alphaDesc{};
    alphaDesc.ByteWidth = sizeof(CBTextureAlpha); // 16バイト境界に合わせたサイズ
    alphaDesc.Usage = D3D11_USAGE_DEFAULT;
    alphaDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    alphaDesc.CPUAccessFlags = 0;
    alphaDesc.MiscFlags = 0;
    alphaDesc.StructureByteStride = 0;

    hr = m_device->CreateBuffer(&alphaDesc, nullptr, m_textureAlphaBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create texture alpha constant buffer");
    }

    //-----------------------シェーダーのコンパイル-----------------------
  
    auto vsBlob = CompileShader(L"BasicVertexShader.hlsl",
		                            "VSMain", "vs_5_0"); 

     hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Basic vertex shader");
    }

    auto psBlob = CompileShader(
        L"BasicPixelShader.hlsl",
        "PSMain",
        "ps_5_0");

    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Basic pixel shader");
    }

    //-----------------------シェーダーオブジェクト作成-----------------------
    m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());

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


    m_device->CreateInputLayout(layoutDesc, _countof(layoutDesc),vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),m_inputLayout.GetAddressOf());

    //-----------------------最後にシェーダー／レイアウトをセット-----------------------
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    //グリッド用の頂点シェーダーをコンパイル
    auto vsGridBlob = CompileShader(
        L"GridVertexShader.hlsl",
        "VSMain",
        "vs_5_0");

    hr = m_device->CreateVertexShader(
        vsGridBlob->GetBufferPointer(),
        vsGridBlob->GetBufferSize(),
        nullptr,
        m_gridVertexShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Grid vertex shader");
    }

    auto psGridBlob = CompileShader(
        L"GridPixelShader.hlsl",
        "PSMain",
        "ps_5_0");

    hr = m_device->CreatePixelShader(
        psGridBlob->GetBufferPointer(),
        psGridBlob->GetBufferSize(),
        nullptr,
        m_gridPixelShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Grid pixel shader");
    }

    // シェーダーオブジェクト作成
    m_device->CreateVertexShader(vsGridBlob->GetBufferPointer(), vsGridBlob->GetBufferSize(), nullptr, m_gridVertexShader.GetAddressOf());
    m_device->CreatePixelShader(psGridBlob->GetBufferPointer(), psGridBlob->GetBufferSize(), nullptr,  m_gridPixelShader.GetAddressOf());

    // --- テクスチャ用シェーダーのコンパイル ---

    auto texVsBlob = CompileShader(
        L"TextureVertexShader.hlsl",
        "VSMain",
        "vs_5_0");

    hr = m_device->CreateVertexShader(
        texVsBlob->GetBufferPointer(),
        texVsBlob->GetBufferSize(),
        nullptr,
        m_textureVertexShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Texture vertex shader");
    }

    
    auto texPsBlob = CompileShader(
        L"TexturePixelShader.hlsl",
        "PSMain",
        "ps_5_0");

    hr = m_device->CreatePixelShader(
        texPsBlob->GetBufferPointer(),
        texPsBlob->GetBufferSize(),
        nullptr,
        m_texturePixelShader.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Texture pixel shader");
    }

    // シェーダーオブジェクト作成
    m_device->CreateVertexShader(texVsBlob->GetBufferPointer(), texVsBlob->GetBufferSize(), nullptr, m_textureVertexShader.GetAddressOf());
    m_device->CreatePixelShader(texPsBlob->GetBufferPointer(), texPsBlob->GetBufferSize(), nullptr, m_texturePixelShader.GetAddressOf());

    // 入力レイアウト（位置(float3) + UV(float2)）
    D3D11_INPUT_ELEMENT_DESC texLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_device->CreateInputLayout(texLayoutDesc, _countof(texLayoutDesc),
        texVsBlob->GetBufferPointer(), texVsBlob->GetBufferSize(), m_textureInputLayout.GetAddressOf());


}


//Direct3Dのリソースは明示的に解放しないとメモリリークが発生するため、
//ComPtr::Reset()で安全にリソースを開放しています。
void Renderer::Uninit()
{
    for (auto& bs : m_blendState)
    {
        bs.Reset();
    }
    m_blendStateATC.Reset();
    m_depthStateEnable.Reset();
    m_depthStateDisable.Reset();
    m_worldBuffer.Reset();
    m_viewBuffer.Reset();
    m_projectionBuffer.Reset();
    m_lightBuffer.Reset();
    m_materialBuffer.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_deviceContext.Reset();
    m_device.Reset();

    char buf[256];
    sprintf_s(buf, "Uninit: device=%p context=%p\n", m_device.Get(), m_pContext.Get());
    OutputDebugStringA(buf);
    m_pContext.Reset();
    m_device.Reset();
}

//画面を指定色（青色）でクリア
//深度バッファも初期化
//毎フレーム必ず呼び出して、前のフレームの残像を消します。
void Renderer::Begin()
{
    ID3D11RenderTargetView* rtv = m_sceneColorRTV.Get();
    m_deviceContext->OMSetRenderTargets(1, &rtv, m_depthStencilView.Get());

    float clearColor[4] = { 0.1f, 0.2f, 0.8f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_sceneColorRTV.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0);
}

void Renderer::End()
{
    ApplyMotionBlur();
    ID3D11RenderTargetView* backRTV = m_renderTargetView.Get();
    m_deviceContext->OMSetRenderTargets(1, &backRTV, nullptr);

    // 深度は不要、アルファ合成を有効にして描画（TransitionManager::Draw は現在の RT に描く想定）
    // 注意：TransitionManager::Draw は内部で SetTextureAlpha などを呼ぶのでここでは単純に呼び出すだけで良い
    TransitionManager::Draw(0.0f); // deltaTime が必要なら適切な値を渡してください

    // 最後に Present
    m_swapChain->Present(1, 0);
}

void Renderer::Present()
{
    m_swapChain->Present(1, 0);
}

void Renderer::SetDepthEnable(bool Enable)
{
    m_deviceContext->OMSetDepthStencilState(
        Enable ? m_depthStateEnable.Get() : m_depthStateDisable.Get(), 0);
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
    m_deviceContext->OMSetBlendState(
        Enable ? m_blendStateATC.Get() : m_blendState[0].Get(),
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
    m_deviceContext->UpdateSubresource(m_worldBuffer.Get(), 0, nullptr, &world, 0, 0);

    Matrix4x4 view = Matrix4x4::Identity.Transpose();
    m_deviceContext->UpdateSubresource(m_viewBuffer.Get(), 0, nullptr, &view, 0, 0);

    Matrix4x4 projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f,
        static_cast<float>(Application::GetWidth()),
        static_cast<float>(Application::GetHeight()),
        0.0f,
        0.0f,
        1.0f);
    projection = projection.Transpose();
    m_deviceContext->UpdateSubresource(m_projectionBuffer.Get(), 0, nullptr, &projection, 0, 0);
}

void Renderer::SetTextureAlpha(float alpha)
{
    CBTextureAlpha cb{};
    cb.Alpha = alpha;
    // UpdateSubresource で CB を更新
    m_deviceContext->UpdateSubresource(m_textureAlphaBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // PixelShader スロット b5 にバインド（ここでバインドするのが安全）
    ID3D11Buffer* cbPtr = m_textureAlphaBuffer.Get();
    m_deviceContext->PSSetConstantBuffers(5, 1, &cbPtr);
}

/**
 * @brief 任意のワールド行列をシェーダーにセットします。
 * @param WorldMatrix ワールド行列へのポインタ
 */
void Renderer::SetWorldMatrix(Matrix4x4* WorldMatrix)
{
    Matrix4x4 mat = WorldMatrix->Transpose();
    m_deviceContext->UpdateSubresource(m_worldBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief 任意のビュー行列をシェーダーにセットします。
 * @param ViewMatrix ビュー行列へのポインタ
 */
void Renderer::SetViewMatrix(SimpleMath::Matrix ViewMatrix)
{
    SimpleMath::Matrix mat = ViewMatrix.Transpose();
    m_deviceContext->UpdateSubresource(m_viewBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief 任意のプロジェクション行列をシェーダーにセットします。
 * @param ProjectionMatrix 射影行列へのポインタ
 */
void Renderer::SetProjectionMatrix(SimpleMath::Matrix ProjectionMatrix)
{
    SimpleMath::Matrix mat = ProjectionMatrix.Transpose();
    m_deviceContext->UpdateSubresource(m_projectionBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief マテリアル（表面材質）情報をセットします。
 * @param Material マテリアル情報
 */
void Renderer::SetMaterial(MATERIAL Material)
{
    m_deviceContext->UpdateSubresource(m_materialBuffer.Get(), 0, nullptr, &Material, 0, 0);
}

/**
 * @brief ライト（光源）情報をセットします。
 * @param Light ライト情報
 */
void Renderer::SetLight(LIGHT Light)
{
    m_deviceContext->UpdateSubresource(m_lightBuffer.Get(), 0, nullptr, &Light, 0, 0);
}

/**
 * @brief 指定したブレンドステートをセットします。
 * @param nBlendState 使用するブレンドステートの種類
 */
void Renderer::SetBlendState(int nBlendState)
{
    if (nBlendState >= 0 && nBlendState < MAX_BLENDSTATE) {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_deviceContext->OMSetBlendState(m_blendState[nBlendState].Get(), blendFactor, 0xffffffff);
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
    HRESULT hr = m_device->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf());
    if (FAILED(hr))
        return;

    m_deviceContext->RSSetState(pRasterizerState.Get());
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
    m_device->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    m_deviceContext->RSSetState(rs.Get());
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
    HRESULT hr = m_device->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        m_deviceContext->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
    }
}

void Renderer::SetPostProcessSettings(const PostProcessSettings& settings)
{
    s_postProcess = settings;
}

const PostProcessSettings& Renderer::GetPostProcessSettings()
{
    return s_postProcess;
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

    m_deviceContext->VSGetShader(&prevVS, nullptr, nullptr);
    m_deviceContext->PSGetShader(&prevPS, nullptr, nullptr);
    m_deviceContext->IAGetInputLayout(&prevIL);
    m_deviceContext->VSGetConstantBuffers(0, 1, prevVSCB);
    m_deviceContext->PSGetConstantBuffers(0, 1, prevPSCB);
    m_deviceContext->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);
    m_deviceContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);
    m_deviceContext->RSGetState(&prevRS);
    m_deviceContext->IAGetPrimitiveTopology(&prevTopo);
    m_deviceContext->IAGetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_deviceContext->PSGetShaderResources(0, 1, prevPSRV);
    m_deviceContext->PSGetSamplers(0, 1, prevSampler);

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
    HRESULT hr = m_device->CreateBuffer(&vbDesc, &initData, vb.GetAddressOf());
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
    m_deviceContext->IASetInputLayout(m_textureInputLayout.Get());
    m_deviceContext->VSSetShader(m_textureVertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_texturePixelShader.Get(), nullptr, 0);
    SetWorldViewProjection2D(); // your helper to set matrices to CBs

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vbPtr = vb.Get();
    m_deviceContext->IASetVertexBuffers(0, 1, &vbPtr, &stride, &offset);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Bind texture + optional sampler (sampler slot 0)
    m_deviceContext->PSSetShaderResources(0, 1, &texture);
    // if you have a sampler object: m_DeviceContext->PSSetSamplers(0,1,m_Sampler.GetAddressOf());

    // Draw
    m_deviceContext->Draw(6, 0);

    // -------- Unbind our SRV to avoid binding it beyond this call --------
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

    // -------- Restore previously saved GPU state --------
    // restore samplers
    m_deviceContext->PSSetSamplers(0, 1, prevSampler);

    // restore SRV
    m_deviceContext->PSSetShaderResources(0, 1, prevPSRV);

    // restore VB
    m_deviceContext->IASetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);

    // restore primitive topology
    m_deviceContext->IASetPrimitiveTopology(prevTopo);

    // restore rasterizer
    m_deviceContext->RSSetState(prevRS);

    // restore depth-stencil
    m_deviceContext->OMSetDepthStencilState(prevDSS, prevStencilRef);

    // restore blend
    m_deviceContext->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);

    // restore constant buffers
    m_deviceContext->VSSetConstantBuffers(0, 1, prevVSCB);
    m_deviceContext->PSSetConstantBuffers(0, 1, prevPSCB);

    // restore shaders & input layout
    m_deviceContext->VSSetShader(prevVS, nullptr, 0);
    m_deviceContext->PSSetShader(prevPS, nullptr, 0);
    m_deviceContext->IASetInputLayout(prevIL);

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
    float blur = std::clamp(s_postProcess.motionBlurAmount, 0.0f, 1.0f);
   
    if (!m_sceneColorTex || !m_sceneColorSRV || !m_prevSceneColorSRV)
    {
        return;
    }


    // BackBuffer のリソース取得（CopyResource に使う）
    ID3D11Resource* backRes = nullptr;
    m_renderTargetView->GetResource(&backRes);

    // ブラー量がほぼゼロ → シーン画像をコピーするだけ
    if (blur <= 0.001f)
    {
        if (backRes)
        {
            // バックバッファにシーン画像をコピー
            m_deviceContext->CopyResource(backRes, m_sceneColorTex.Get());

            // 次フレーム用に prev にもコピー
            m_deviceContext->CopyResource(m_prevSceneColorTex.Get(), m_sceneColorTex.Get());

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

    m_deviceContext->VSGetShader(&prevVS, nullptr, nullptr);
    m_deviceContext->PSGetShader(&prevPS, nullptr, nullptr);
    m_deviceContext->IAGetInputLayout(&prevIL);
    m_deviceContext->VSGetConstantBuffers(0, 1, prevVSCB);
    m_deviceContext->PSGetConstantBuffers(0, 1, prevPSCB);
    m_deviceContext->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);
    m_deviceContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);
    m_deviceContext->RSGetState(&prevRS);
    m_deviceContext->IAGetPrimitiveTopology(&prevTopo);
    m_deviceContext->IAGetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_deviceContext->PSGetShaderResources(0, 2, prevPSRV);
    m_deviceContext->PSGetSamplers(0, 1, prevSampler);

    // ================================
    // バックバッファへ描画する
    // ================================
    ID3D11RenderTargetView* backRTV = m_renderTargetView.Get();
    m_deviceContext->OMSetRenderTargets(1, &backRTV, nullptr);

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

    HRESULT hr = m_device->CreateBuffer(&vbDesc, &init, vb.GetAddressOf());
    if (FAILED(hr))
    {
        if (backRes) backRes->Release();
        return;
    }

    // ================================
    // MotionBlur シェーダー設定
    // ================================
    m_deviceContext->IASetInputLayout(m_textureInputLayout.Get());
    m_deviceContext->VSSetShader(m_textureVertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_motionBlurPixelShader.Get(), nullptr, 0);

    SetWorldViewProjection2D();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vbPtr = vb.Get();
    m_deviceContext->IASetVertexBuffers(0, 1, &vbPtr, &stride, &offset);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ================================
    // SRV をセット（SceneTex & PrevTex）
    // ================================
    ID3D11ShaderResourceView* srvs[2] =
    {
        m_sceneColorSRV.Get(),      // t0
        m_prevSceneColorSRV.Get(),  // t1
    };
    m_deviceContext->PSSetShaderResources(0, 2, srvs);

    // ================================
    // ★ 定数バッファ更新（最重要ポイント）★
    // ================================
    PostProcessSettings cbData = s_postProcess;
    cbData.motionBlurAmount = blur;  // クランプ済みに書き換え

    m_deviceContext->UpdateSubresource(
        m_postProcessBuffer.Get(), 0, nullptr, &cbData, 0, 0
    );

    ID3D11Buffer* ppCB = m_postProcessBuffer.Get();
    m_deviceContext->PSSetConstantBuffers(0, 1, &ppCB);

    // ================================
    // 描画
    // ================================
    m_deviceContext->Draw(6, 0);

    // SRV 解放
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    m_deviceContext->PSSetShaderResources(0, 2, nullSRV);

    // ================================
    // GPU のステートをすべて復元
    // ================================
    m_deviceContext->PSSetSamplers(0, 1, prevSampler);
    m_deviceContext->PSSetShaderResources(0, 2, prevPSRV);
    m_deviceContext->IASetVertexBuffers(0, 1, prevVBs, prevStrides, prevOffsets);
    m_deviceContext->IASetPrimitiveTopology(prevTopo);
    m_deviceContext->RSSetState(prevRS);
    m_deviceContext->OMSetDepthStencilState(prevDSS, prevStencilRef);
    m_deviceContext->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);
    m_deviceContext->VSSetConstantBuffers(0, 1, prevVSCB);
    m_deviceContext->PSSetConstantBuffers(0, 1, prevPSCB);
    m_deviceContext->VSSetShader(prevVS, nullptr, 0);
    m_deviceContext->PSSetShader(prevPS, nullptr, 0);
    m_deviceContext->IASetInputLayout(prevIL);

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
        m_deviceContext->CopyResource(m_prevSceneColorTex.Get(), m_sceneColorTex.Get());
        backRes->Release();
    }
}

void Renderer::DrawReticle(ID3D11ShaderResourceView* texture, const POINT& center, const Vector2& size)
{
    if (!texture) return;

    // Save/restore depth & blend state quickly (we'll use DrawTexture which restores most state)
    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencilRef = 0;
    m_deviceContext->OMGetDepthStencilState(&prevDSS, &prevStencilRef);

    ID3D11BlendState* prevBlend = nullptr;
    FLOAT prevBlendFactor[4] = { 0,0,0,0 };
    UINT prevSampleMask = 0xFFFFFFFF;
    m_deviceContext->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);

    // turn off depth, enable alpha blend (assume you have BS_ALPHABLEND created and stored)
    SetDepthEnable(false);
    SetBlendState(BS_ALPHABLEND);

    Vector2 topLeft;
    topLeft.x = static_cast<float>(center.x) - size.x * 0.5f;
    topLeft.y = static_cast<float>(center.y) - size.y * 0.5f;
    DrawTexture(texture, topLeft, size);

    // restore blend/depth (DrawTexture already restores shaders/IL etc.)
    // but if SetBlendState/SetDepthEnable modified device state outside DrawTexture, restore here:
    m_deviceContext->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);
    m_deviceContext->OMSetDepthStencilState(prevDSS, prevStencilRef);

    if (prevDSS) prevDSS->Release();
    if (prevBlend) prevBlend->Release();
}

void Renderer::BeginSceneRenderTarget()
{
    ID3D11RenderTargetView* rtv = m_sceneColorRTV.Get();
    ID3D11DepthStencilView* dsv = m_depthStencilView.Get();

    // シーン用RTV + 深度
    m_deviceContext->OMSetRenderTargets(1, &rtv, dsv);

    float clearColor[4] = { 0, 0, 0, 1 };
    m_deviceContext->ClearRenderTargetView(rtv, clearColor);
    m_deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // シーン用ステート
    m_deviceContext->RSSetState(m_rasterizerState.Get());
    m_deviceContext->RSSetViewports(1, &m_viewport);
}


void Renderer::BeginBackBuffer()
{
    ID3D11RenderTargetView* rtv = m_renderTargetView.Get();
    m_deviceContext->OMSetRenderTargets(1, &rtv, nullptr);

    float clearColor[4] = { 0, 0, 0, 1 };
    m_deviceContext->ClearRenderTargetView(rtv, clearColor);

    // UI では Z は不要なので深度は無し
    // ラスタライザとビューポートもデフォルトで OK
}





//m_DeviceContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);