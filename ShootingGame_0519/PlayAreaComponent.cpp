#include "PlayAreaComponent.h"
#include "GameScene.h"
#include "RaycastHit.h" 
#include <algorithm>

using namespace DirectX::SimpleMath;

PlayAreaComponent::PlayAreaComponent()
{
}

float PlayAreaComponent::GetGroundHeightAt(const Vector3& p) const
{
    // 簡易版: 平坦地を返す。
    // 将来的には m_scene を使って地形（heightmap）や地面コライダにレイキャストする実装にできます。
    return m_groundY;
}

Vector3 PlayAreaComponent::ResolvePosition(const Vector3& prevPos,
    const Vector3& desiredPos,
    GameObject* owner) const
{
    Vector3 out = desiredPos;

    //AABB 内に閉じる（X/Z と Y 両方）
    if (out.x < m_boundsMin.x) { out.x = m_boundsMin.x; }
    if (out.y < m_boundsMin.y) { out.y = m_boundsMin.y; }
    if (out.z < m_boundsMin.z) { out.z = m_boundsMin.z; }

    if (out.x > m_boundsMax.x) { out.x = m_boundsMax.x; }
    if (out.y > m_boundsMax.y) { out.y = m_boundsMax.y; }
    if (out.z > m_boundsMax.z) { out.z = m_boundsMax.z; }

    // 2) 地面にぶつける（Y を地面より下にしない）
    float groundH = GetGroundHeightAt(out);
    if (out.y < groundH)
    {
        out.y = groundH;
    }
    return out;
}

