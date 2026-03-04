#include "ForwardMoveComponent.h"
#include "GameObject.h"

static Vector3 SafeNormalize(const Vector3& v, const Vector3& fallback)
{
	if (v.LengthSquared() <= 0.000001f)
	{
		return fallback;
	}

	Vector3 n = v;
	n.Normalize();
	return n;
}

void ForwardMoveComponent::Initialize()
{
	if (!GetOwner()){ return; }

	m_railCenterPos = GetOwner()->GetPosition();

	m_railForward = SafeNormalize(m_railForward, Vector3::Forward);
	m_railUp = SafeNormalize(m_railUp, Vector3::Up);

	m_railRight = m_railForward.Cross(m_railUp);
	m_railRight = SafeNormalize(m_railRight, Vector3::Right);
}

void ForwardMoveComponent::Update(float delta)
{
	if (!GetOwner()){ return; }

	//軸の安全化（曲がり対応した時に崩れないように）
	m_railForward = SafeNormalize(m_railForward, Vector3::Forward);
	m_railUp = SafeNormalize(m_railUp, Vector3::Up);

	m_railRight = m_railForward.Cross(m_railUp);
	m_railRight = SafeNormalize(m_railRight, Vector3::Right);

	//レール中心を更新（今は “前進した位置＝中心” でOK）
	m_railCenterPos += m_railForward * m_speed * delta;

	//現段階はそのまま Owner に反映
	//GetOwner()->SetPosition(m_railCenterPos);
}

void ForwardMoveComponent::Uninit()
{

}