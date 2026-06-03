#include "GameObject.h"
#include "OBBColliderComponent.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

void OBBColliderComponent::SetSize(const Vector3& size)
{
    m_size = size;
}

//GameObjectの位置のゲット関数
Vector3 OBBColliderComponent::GetCenter() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return Vector3::Zero;
    }

    // オフセットをスケール（ローカル->ワールドでの大きさの差を反映）
    Vector3 scaledOffset = Vector3(m_localOffset.x * owner->GetScale().x,
        m_localOffset.y * owner->GetScale().y,
        m_localOffset.z * owner->GetScale().z);

    // 回転（オブジェクトの向き）によりローカルオフセットをワールドに回す
    Vector3 rot = owner->GetRotation();
    Matrix rotMat = Matrix::CreateFromYawPitchRoll(rot.y, rot.x, rot.z);

    Vector3 worldOffset = Vector3::Transform(scaledOffset, rotMat);

    // ワールド中心 = オーナー位置 + ワールドオフセット
    return owner->GetPosition() + worldOffset;
};

//当たり判定用のサイズのゲット関数(m_size)
Vector3 OBBColliderComponent::GetSize() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return m_size; // フォールバック
    }
    Vector3 s = owner->GetScale();
    return Vector3(m_size.x * s.x, m_size.y * s.y, m_size.z * s.z);
}

//ゲームオブジェクトの回転値から、回転行列を生成。
DirectX::SimpleMath::Matrix OBBColliderComponent::GetRotationMatrix() const
{
    GameObject* owner = GetOwner();
    if (!owner)
    {
        return DirectX::SimpleMath::Matrix::Identity;
    }
    Vector3 rot = owner->GetRotation();
    return DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(rot.y, rot.x, rot.z);
}