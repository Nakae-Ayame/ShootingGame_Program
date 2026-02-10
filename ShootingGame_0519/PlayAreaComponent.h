 #pragma once
#include "RaycastHit.h"
#include "GameScene.h"
#include <SimpleMath.h>
#include <functional>

class PlayAreaComponent : public Component
{
public:
    PlayAreaComponent();

    void Initialize() override {}
    void Update(float dt) override {}

    // シーン参照をセット
    void SetScene(GameScene* scene) { m_scene = scene; }

    // ワールド領域（AABB）
    void SetBounds(const DirectX::SimpleMath::Vector3& min, const DirectX::SimpleMath::Vector3& max)
    {
        m_boundsMin = min;
        m_boundsMax = max;
    }

    // 単純地面（平坦）
    void SetGroundY(float y) { m_groundY = y; }

    float GetGroundHeightAt(const DirectX::SimpleMath::Vector3& p) const;

    const DirectX::SimpleMath::Vector3& GetBoundsMin() const { return m_boundsMin; }

    const DirectX::SimpleMath::Vector3& GetBoundsMax() const { return m_boundsMax; }

    // ResolvePosition: prevPos->desiredPos をプレイルールに従って補正
    DirectX::SimpleMath::Vector3 ResolvePosition(const DirectX::SimpleMath::Vector3& prevPos,
        const DirectX::SimpleMath::Vector3& desiredPos,
        GameObject* owner = nullptr) const;

private:
    GameScene* m_scene = nullptr;

    DirectX::SimpleMath::Vector3 m_boundsMin = DirectX::SimpleMath::Vector3(-1000, -1000, -1000);
    DirectX::SimpleMath::Vector3 m_boundsMax = DirectX::SimpleMath::Vector3(1000, 1000, 1000);

    float m_groundY = -7.0f; // デフォルトの地面高さ（平坦）
};
