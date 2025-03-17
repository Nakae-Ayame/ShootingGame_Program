#pragma once

#include	<d3d11.h>
#include	<DirectXMath.h>
#include	<SimpleMath.h>
#include	<io.h>
#include	<string>
#include	<vector>

//外部ライブラリ
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

// ３Ｄ頂点データ
struct VERTEX_3D
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 normal;
	DirectX::SimpleMath::Color color;
	DirectX::SimpleMath::Vector2 uv;
};

// ブレンドステート
enum EBlendState {
	BS_NONE = 0,							// 半透明合成無し
	BS_ALPHABLEND,							// 半透明合成
	BS_ADDITIVE,							// 加算合成
	BS_SUBTRACTION,							// 減算合成
	MAX_BLENDSTATE
};

struct LIGHT
{
	BOOL Enable;
	BOOL Dummy[3];//バイト数的にキリを付けるために入れている
	DirectX::SimpleMath::Vector4 Direction;//平行光源の方向
	DirectX::SimpleMath::Color Diffuse;//並行光源の強さと色
	DirectX::SimpleMath::Color Ambient;//環境光の強さと色
};

//サブセット
struct SUBSET
{
	std::string MtrlName;
	unsigned int IndexNum = 0;    //material名
	unsigned int VertexNum = 0;   //index数
	unsigned int IndexBase = 0;   //頂点数
	unsigned int VertexBase = 0;  //開始index
	unsigned int MaterialIdx = 0; //materialの番号

};

// マテリアル
struct MATERIAL 
{ 
	DirectX::SimpleMath::Color Ambient; //環境反射
	DirectX::SimpleMath::Color Diffuse; //拡散反射（≒カラー） 
	DirectX::SimpleMath::Color Specular; //鏡面反射 
	DirectX::SimpleMath::Color Emission; //発光 
	float Shiness; //光沢の滑らかさ 
	BOOL TextureEnable; //テクスチャを使うか否かのフラグ 
	BOOL Dummy[2];
};

//-----------------------------------------------------------------------------
//Rendererクラス
//-----------------------------------------------------------------------------
class Renderer
{
private:

	static D3D_FEATURE_LEVEL       m_FeatureLevel;

	static ID3D11Device*           m_Device;
	static ID3D11DeviceContext*    m_DeviceContext;
	static IDXGISwapChain*         m_SwapChain;
	static ID3D11RenderTargetView* m_RenderTargetView;
	static ID3D11DepthStencilView* m_DepthStencilView;

	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;

	static ID3D11Buffer* m_LightBuffer;
	static ID3D11Buffer* m_MaterialBuffer;
	static ID3D11Buffer* m_TextureBuffer; //UV


	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	static ID3D11BlendState*		m_BlendState[MAX_BLENDSTATE]; // ブレンド ステート;
	static ID3D11BlendState*		m_BlendStateATC;

public:

	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	static void SetDepthEnable(bool Enable);

	static void SetATCEnable(bool Enable);

	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(DirectX::SimpleMath::Matrix* WorldMatrix);
	static void SetViewMatrix(DirectX::SimpleMath::Matrix* ViewMatrix);
	static void SetProjectionMatrix(DirectX::SimpleMath::Matrix* ProjectionMatrix);

	static ID3D11Device* GetDevice( void ){ return m_Device; }
	static ID3D11DeviceContext* GetDeviceContext( void ){ return m_DeviceContext; }

	static void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	static void SetLight(LIGHT Light);
	static void SetMaterial(MATERIAL Material);
	static void SetUV(float u, float v, float uw, float vh);

	//=============================================================================
	// ブレンド ステート設定
	//=============================================================================
	static void SetBlendState(int nBlendState)
	{
		if (nBlendState >= 0 && nBlendState < MAX_BLENDSTATE) {
			float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_DeviceContext->OMSetBlendState(m_BlendState[nBlendState], blendFactor, 0xffffffff);
		}
	}
};