#include "SphereColliderComponent.h"

#include "GameObject.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

Vector3 SphereColliderComponent::GetCenter() const
{
	GameObject* owner = GetOwner();
	if (!owner)
	{
		return Vector3::Zero;
	}
	//ローカルオフセット * オブジェクトのサイズ
	Vector3 scaledOffset = Vector3(m_localOffset.x * owner->GetScale().x,
								   m_localOffset.y * owner->GetScale().y,
								   m_localOffset.z * owner->GetScale().z);

	Vector3 rot = owner->GetRotation();
	Matrix rotMat = Matrix::CreateFromYawPitchRoll(rot.y, rot.x, rot.z);
	Vector3 worldOffset = Vector3::Transform(scaledOffset, rotMat);

	return owner->GetPosition() + worldOffset;
}

Matrix SphereColliderComponent::GetRotationMatrix() const
{
	return Matrix::Identity;
}

