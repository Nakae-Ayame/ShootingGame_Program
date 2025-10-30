#include "AABBColliderComponent.h"
#include "GameObject.h"

//GameObjectの位置のゲット関数
Vector3 AABBColliderComponent::GetCenter() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return Vector3::Zero;
    }
    return owner->GetPosition();
}

//当たり判定用のサイズのゲット関数(m_Size)
Vector3 AABBColliderComponent::GetSize() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return m_Size;// ローカルサイズを返すフォールバック
    }
    return Vector3(m_Size.x * owner->GetScale().x,
                   m_Size.y * owner->GetScale().y,
                   m_Size.z * owner->GetScale().z);
}

//AABBは回転を考慮しないのでIdentity(回転なし)にしておく
DirectX::SimpleMath::Matrix AABBColliderComponent::GetRotationMatrix() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return DirectX::SimpleMath::Matrix::Identity;
    }
    Vector3 rot = owner->GetRotation();
    return DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(rot.y, rot.x, rot.z);
}

//中央から半分のサイズを引いて最小座標を出す
Vector3 AABBColliderComponent::GetMin() const
{
    return GetCenter() - m_Size * 0.5f;
}

//中央から半分のサイズを引いて最大座標を出す
Vector3 AABBColliderComponent::GetMax() const
{
    return GetCenter() + m_Size * 0.5f;
}