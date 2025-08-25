#pragma once
#include "Component.h"
#include "commontypes.h"

enum class ColliderType
{
    AABB,
    OBB,
};

class ColliderComponent : public Component
{
public:
    ColliderComponent(ColliderType type): m_Type(type) {}

    virtual ~ColliderComponent() = default;

    ColliderType GetColliderType() const { return m_Type; }

    // 共通インターフェース追加
    virtual Vector3 GetCenter() const = 0;
    /*virtual Vector3 GetMin() const = 0;
    virtual Vector3 GetMax() const = 0;*/
    virtual Vector3 GetSize() const = 0;
    virtual DirectX::SimpleMath::Matrix GetRotationMatrix() const = 0;

    void SetHitThisFrame(bool hit) { m_hitThisFrame = hit; }
    bool IsHitThisFrame() const { return m_hitThisFrame; }

protected:
    ColliderType m_Type;
    bool m_hitThisFrame = false; // 毎フレームの衝突状態
};
