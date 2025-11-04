#pragma once
#include "Component.h"
#include "Primitive.h"
#include "AABBColliderComponent.h"
#include "TextureManager.h"
#include "CollisionManager.h"
#include "renderer.h"
#include <SimpleMath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

//--------------------------------------
//床のオブジェクトのコンポーネントクラス
//--------------------------------------
class FloorComponent : public Component
{
public:

    //グリットの種類
    enum class GridMode 
    {
        Lines,     //線を入れるか
        Textured   //テクスチャを貼るか
    };

    void Initialize() override;
    //void Update(float dt) override {}
    void Draw(float alpha) override;

    //サイズのゲッター
    float GetWidth() const { return m_width; }
    float GetDepth() const { return m_depth; }

    void SetSize(float width, float depth) { m_width = width; m_depth = depth; }
    void SetWidth(float width) { m_width = width; }
    void SetDepth(float depth) { m_depth = depth;  }
    void SetYOffset(float y) { m_y = y; }
    void SetGridStep(float step) { m_gridStep = step; }
    void SetColor(const Vector4& c) { m_color = c; }

    void SetGridTexture(const std::string& path, float tileU = 1.0f, float tileV = 1.0f)
    {
        m_mode = GridMode::Textured;
        m_texture = path;
        m_tileU = tileU; m_tileV = tileV;
    }

private:

    
    GridMode m_mode = GridMode::Textured;
    float m_width = 10.0f;      //幅
    float m_depth = 10.0f;      //奥行き
    float m_y = 0.0f;           //床の高さ位置
    float m_gridStep = 1.0f;    //グリッド線の間隔
    Vector4 m_color = Vector4(0.6f, 0.6f, 0.6f, 1.0f);  //グリッドの色

    std::string m_texture;
    float m_tileU = 1.0f;
    float m_tileV = 1.0f;
    ID3D11ShaderResourceView* m_gridSRV = nullptr;

    Primitive m_prim;           //箱を作るクラス
    std::shared_ptr<AABBColliderComponent> m_collider; // 所有はGameObject側のvectorにあるが便宜上保持
};
