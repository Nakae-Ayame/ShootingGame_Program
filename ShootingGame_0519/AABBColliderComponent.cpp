#include "AABBColliderComponent.h"
#include "GameObject.h"

//GameObjectの位置のゲット関数
Vector3 AABBColliderComponent::GetCenter() const
{
    return GetOwner()->GetPosition();
}

//当たり判定用のサイズのゲット関数(m_Size)
Vector3 AABBColliderComponent::GetSize() const
{
    return m_Size;
}

//AABBは回転を考慮しないのでIdentity(回転なし)にしておく
DirectX::SimpleMath::Matrix AABBColliderComponent::GetRotationMatrix() const
{
    return DirectX::SimpleMath::Matrix::Identity;
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