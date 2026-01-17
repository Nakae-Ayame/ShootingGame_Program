#pragma once
#include <d3d11.h>
#include <io.h>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <SimpleMath.h>
#include "CommonTypes.h"
#include "Transform.h"
#include "VisualSettings.h"
#include "Sound.h"

using namespace DirectX;

 // リンクすべき外部ライブラリ
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

 //ボーンの影響情報を保持する構造体
struct WEIGHT
{
    std::string bonename;   //ボーン名
    std::string meshname;   //メッシュ名
    float weight;           //ウェイト値
    int vertexindex;        //頂点インデックス
};

//ボーン構造体（DX対応版）
struct BONE
{
    std::string bonename;          //ボーン名
    std::string meshname;          //メッシュ名
    std::string armaturename;      //アーマチュア名
    Matrix4x4 Matrix{};            //親子関係を考慮した行列
    Matrix4x4 AnimationMatrix{};   //自分の変形のみを考慮した行列
    Matrix4x4 OffsetMatrix{};      //ボーンオフセット行列
    int idx;                       //配列中のインデックス
    std::vector<WEIGHT> weights;   //このボーンが影響を与える頂点とウェイト値のリスト
};


//３次元頂点データを格納する構造体
struct VERTEX_3D
{
    Vector3 Position;            //頂点の座標
    Vector3 Normal;              //法線ベクトル
    Color Diffuse;               //拡散反射色
    Vector2 TexCoord;            //テクスチャ座標
    int BoneIndex[4];            //ボーンインデックス（最大4つ） 20231225
    float BoneWeight[4];         //各ボーンのウェイト値 20231225
    //std::string BoneName[4];     //各ボーンの名前 20231226
    int bonecnt = 0;             //影響を与えるボーン数 20231226
};


//マテリアル情報を保持する構造体
struct MATERIAL
{
    Color Ambient;         //アンビエント色
    Color Diffuse;         //拡散色
    Color Specular;        //鏡面反射色
    Color Emission;        //自己発光色
    float Shiness;         //光沢度
    BOOL TextureEnable;    //テクスチャ使用フラグ
    float Dummy[2]{};      //予備領域
};

//平行光源の情報を保持する構造体
struct LIGHT
{
    BOOL Enable;           //ライトの有効/無効フラグ
    BOOL Dummy[3];         //パディング用（ダミー）
    Vector4 Direction;     //光の方向
    Color Diffuse;         //拡散光の色
    Color Ambient;         //環境光の色
};

//メッシュのサブセット（マテリアル毎）情報を保持する構造体
struct SUBSET
{
    std::string MtrlName;           //マテリアル名
    unsigned int IndexNum = 0;      //インデックス数
    unsigned int VertexNum = 0;     //頂点数
    unsigned int IndexBase = 0;     //開始インデックス
    unsigned int VertexBase = 0;    //頂点ベース
    unsigned int MaterialIdx = 0;   //マテリアルインデックス
};

//ブレンドステートの種類
enum EBlendState
{
    BS_NONE = 0,      //半透明合成無し
    BS_ALPHABLEND,    //半透明合成
    BS_ADDITIVE,      //加算合成
    BS_SUBTRACTION,   //減算合成
    MAX_BLENDSTATE    //ブレンドステートの最大値
};

//ボーンコンビネーション行列を保持する構造体
constexpr int MAX_BONE = 400;
struct CBBoneCombMatrix
{
    DirectX::XMFLOAT4X4 BoneCombMtx[MAX_BONE];  ///< ボーンコンビネーション行列の配列
};

struct CBTextureAlpha
{
    float Alpha;
    float Padding[3];
};

// @brief DirectXレンダリング処理を管理するレンダラクラス
//このクラスは、Direct3Dデバイス、コンテキスト、スワップチェーンなどの管理と、
//レンダリング処理の初期化、開始、終了などの機能を提供します。

class Renderer// : NonCopyable
{
private:
    static D3D_FEATURE_LEVEL m_featureLevel;

    static ComPtr<ID3D11Device> m_device;
    static ComPtr<ID3D11DeviceContext> m_deviceContext;
    static ComPtr<IDXGISwapChain> m_swapChain;
    static ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    static ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    static ComPtr<ID3D11Buffer> m_worldBuffer;
    static ComPtr<ID3D11Buffer> m_viewBuffer;
    static ComPtr<ID3D11Buffer> m_projectionBuffer;
    static ComPtr<ID3D11Buffer> m_materialBuffer;
    static ComPtr<ID3D11Buffer> m_lightBuffer;

    static ComPtr<ID3D11DepthStencilState> m_depthStateEnable;
    static ComPtr<ID3D11DepthStencilState> m_depthStateDisable;

    static ComPtr<ID3D11BlendState> m_blendState[MAX_BLENDSTATE];
    static ComPtr<ID3D11BlendState> m_blendStateATC;

    static int m_indexCount;

    //-------------------------------ポストプロセス用------------------------------
    static PostProcessSettings s_postProcess;

    static Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_sceneColorTex;    
    static Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_sceneColorRTV;
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sceneColorSRV;

    static Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_prevSceneColorTex;
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_prevSceneColorSRV;

    static Microsoft::WRL::ComPtr<ID3D11PixelShader>        m_motionBlurPixelShader;
    static Microsoft::WRL::ComPtr<ID3D11Buffer>             m_postProcessBuffer;

    //-----------------------------------------------------------------------------

    //シェーダコンパイルの共通ヘルパ
    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const wchar_t* filePath,
                                                          const char* entryPoint,
                                                          const char* target);

public:
    static Renderer& Get();
    static void Init();
    static void Uninit();
    static void Begin();
    static void End();
	static void Present();

	static void BeginSceneRenderTarget();
	static void BeginBackBuffer();
    static void ApplyMotionBlur(); // End() から呼ぶ内部処理



    //-----------------------Set関数関連--------------------------------------
    static void SetDepthEnable(bool Enable);
    static void SetDepthAllwaysWrite();
    static void SetATCEnable(bool Enable);
    static void SetWorldViewProjection2D();
    static void SetWorldMatrix(Matrix4x4* WorldMatrix);
    static void SetViewMatrix(SimpleMath::Matrix ViewMatrix);
    static void SetProjectionMatrix(SimpleMath::Matrix ProjectionMatrix);
    static void SetMaterial(MATERIAL Material);
    static void SetLight(LIGHT Light);
    static void SetTexture(ID3D11ShaderResourceView* texture){m_deviceContext->PSSetShaderResources(0, 1, &texture);}
    static ID3D11Device* GetDevice(void) { return m_device.Get(); }
    static ID3D11DeviceContext* GetDeviceContext(void) { return m_deviceContext.Get(); }
    static void SetBlendState(int nBlendState);
    static IDXGISwapChain* GetSwapChain() { return m_swapChain.Get(); }
    static void ClearDepthBuffer()
    {
        m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    static void DisableCulling(bool cullflag = false);
    static void SetFillMode(D3D11_FILL_MODE FillMode);
    static int GetIndexCount() { return m_indexCount; }
    static ID3D11Buffer* GetViewBuffer(){ return m_viewBuffer.Get(); }
    static ID3D11Buffer* GetWorldBuffer(){ return m_worldBuffer.Get(); }
    static ID3D11Buffer* GetProjectionBuffer(){ return m_projectionBuffer.Get(); }

    static void SetPostProcessSettings(const PostProcessSettings& settings);
    static const PostProcessSettings& GetPostProcessSettings();

    static void SetTextureAlpha(float alpha);

    //レティクル用の関数
    static void DrawReticle(ID3D11ShaderResourceView* texture, const POINT& center, const Vector2& size);

    static void DrawBillboard(ID3D11ShaderResourceView* texture,
                              const DirectX::SimpleMath::Vector3& worldPos,
                              float size,const DirectX::SimpleMath::Vector4& color,
                              int cols = 1,int rows = 1,int frameIndex = 0,bool isAdditive = true);

    // マテリアル用定数バッファのポインタを取得
    static ID3D11Buffer* GetMaterialCB()
    {
        return m_materialBuffer.Get();
    }

    // マテリアル用定数バッファのアドレス（ID3D11Buffer**）を取得
    static ID3D11Buffer** GetMaterialCBAddress()
    {
        return m_materialBuffer.GetAddressOf();
    }

    static void DrawTexture(ID3D11ShaderResourceView* texture, const Vector2& position, const Vector2& size);

    //------------------------------Billboard関連------------------------------
    static ComPtr<ID3D11VertexShader> m_billboardVertexShader;
    static ComPtr<ID3D11PixelShader>  m_billboardPixelShader;
    static ComPtr<ID3D11InputLayout>  m_billboardInputLayout;

    // 「今最後にセットされたView/Proj」をRenderer側で覚える（ビルボードの向き計算に必要）
    static DirectX::SimpleMath::Matrix m_cachedView;
    static DirectX::SimpleMath::Matrix m_cachedProjection;

    static ComPtr<ID3D11VertexShader> m_vertexShader;
    static ComPtr<ID3D11PixelShader>  m_pixelShader;
    static ComPtr<ID3D11InputLayout>  m_inputLayout;
    static ComPtr<ID3D11InputLayout>  m_axisInputLayout;

    //テクスチャ描画用のシェーダーとレイアウト
    static ComPtr<ID3D11VertexShader> m_textureVertexShader;
    static ComPtr<ID3D11PixelShader>  m_texturePixelShader;
    static ComPtr<ID3D11InputLayout>  m_textureInputLayout;
    static ComPtr<ID3D11Buffer>       m_textureAlphaBuffer;

    static D3D11_VIEWPORT m_viewport;                // シーン描画用ビューポート
    static Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState; // 標準ラスタライザ

    // Grid専用のシェーダー
    static ComPtr<ID3D11VertexShader> m_gridVertexShader;
    static ComPtr<ID3D11PixelShader>  m_gridPixelShader;

    static ComPtr<ID3D11DeviceContext>  m_pContext; // 初期化済みのデバイスコンテキスト
    static ComPtr<ID3D11BlendState>     m_pBlendState; // アルファブレンド用
    static ComPtr<ID3D11Buffer>         m_pVertexBuffer; // フルスクリーン用頂点バッファ
};