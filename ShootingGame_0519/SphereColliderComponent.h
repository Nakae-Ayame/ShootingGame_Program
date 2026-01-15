#pragma once
#include "ColliderComponent.h"
#include <SimpleMath.h>
#include <DirectXMath.h>

class SphereColliderComponent : public ColliderComponent
{
public:
	SphereColliderComponent() : ColliderComponent(ColliderType::SPHERE) {}
	~SphereColliderComponent() override {};

	//-------------Setä÷êî--------------
	void SetRadius(float radius) { m_radius = radius; };
	void SetLocalOffset(const DirectX::SimpleMath::Vector3& offset) { m_localOffset = offset; };

	//-------------Getä÷êî--------------
	float GetRadius() const { return m_radius; };
	const Vector3& GetLocalOffset() const { return  m_localOffset; };
	Vector3 GetCenter() const override;
	Vector3 GetSize() const override { return DirectX::SimpleMath::Vector3::Zero; };
	DirectX::SimpleMath::Matrix GetRotationMatrix() const override;

private:
	//--------------Sphereä÷òA------------------
	float m_radius = 1.0f;
	Vector3 m_localOffset = Vector3::Zero;
};