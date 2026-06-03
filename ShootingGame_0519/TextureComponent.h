#pragma once
#include "Component.h"
#include <string>
#include <SimpleMath.h>
#include <wrl/client.h>
#include <d3d11.h>

class TextureComponent : public Component
{
public:
    //コンストラクタ・デストラクタ
    TextureComponent();
    ~TextureComponent() override = default;

    //初期化
    void Initialize() override;

    //描画
    void Draw(float alpha) override;

    //テクスチャ読み込み
    bool LoadTexture(const std::wstring& filepath);

    //画面(UnityのCanvasみたいな)上の座標のセット関数(左上基準)
    void SetScreenPosition(float x, float y) { m_position = { x,y };}

    //サイズのセット関数
    void SetSize(float width, float height) { m_size = { width ,height }; }

    void SetVisible(bool isVisible) { m_isVisible = isVisible; }

	void SetAlpha(float alpha) { m_alpha = alpha; }

    //--------Get関数-------
    bool GetVisible() const { return m_isVisible; }
    ID3D11ShaderResourceView* GetSRV() const { return m_textureSrv.Get(); }

private:
    //Texture保存変数
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSrv;

    //画面上の座標の変数
    DirectX::SimpleMath::Vector2 m_position;

    //Textureのサイズの変数
    DirectX::SimpleMath::Vector2 m_size;

	float m_alpha = 1.0f;

    //--------------表示関連------------------
    bool m_isVisible = true;
};