#pragma once
#include "Component.h"
#include <SimpleMath.h>
#include <d3d11.h>

class BulletTrailComponent : public Component
{
public:
    BulletTrailComponent() = default;
	~BulletTrailComponent() override = default;

	void Initialize() override;
	void Update(float dt) override;
	void Draw(float alpha) override;

    //--------Set関数-------
    void SetSegment(const DirectX::SimpleMath::Vector3& startPos, const DirectX::SimpleMath::Vector3& endPos);
    void SetWidth(float w) { m_width = w; }
    void SetDuration(float d) { m_duration = d; }
    void SetColor(const DirectX::SimpleMath::Vector4& c) { m_color = c; }
    void SetTexture(ID3D11ShaderResourceView* srv) { m_textureSrv = srv; }
    void SetAdditive(bool add) { m_isAdditive = add; }

    //--------Get関数-------
    bool IsFinished() const { return m_isFinished; }

private:
    //--------------寿命関連------------------
    float m_elapsed = 0.0f;
    float m_duration = 0.15f;
    bool m_isFinished = false;

    //--------------形状関連------------------
    DirectX::SimpleMath::Vector3 m_startPos = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 m_endPos = DirectX::SimpleMath::Vector3::Zero;

    //--------------見た目関連------------------
    float m_width = 0.35f;
    DirectX::SimpleMath::Vector4 m_color = { 0.2f, 1.0f, 1.0f, 0.25f };
    ID3D11ShaderResourceView* m_textureSrv = nullptr;
    bool m_isAdditive = true;
};