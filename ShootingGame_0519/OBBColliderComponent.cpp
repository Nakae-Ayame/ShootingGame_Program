#include "GameObject.h"
#include "OBBColliderComponent.h"

void OBBColliderComponent::SetSize(const Vector3& size)
{
    m_Size = size;
}

//GameObjectの位置のゲット関数
Vector3 OBBColliderComponent::GetCenter() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return Vector3::Zero;
    }
    return owner->GetPosition();
};

//当たり判定用のサイズのゲット関数(m_Size)
Vector3 OBBColliderComponent::GetSize() const
{
    return Vector3(m_Size.x * GetOwner()->GetScale().x,
        m_Size.y * GetOwner()->GetScale().y,
        m_Size.z * GetOwner()->GetScale().z);
}

//ゲームオブジェクトの回転値から、回転行列を生成。
DirectX::SimpleMath::Matrix OBBColliderComponent::GetRotationMatrix() const
{
    Vector3 rot = GetOwner()->GetRotation();
    return DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(rot.y, rot.x, rot.z);
}