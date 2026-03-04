#include "ScreenMoveComponent.h"
#include "Input.h"
#include "GameObject.h"
#include "Application.h"

void ScreenMoveComponent::Initialize()
{

}

void ScreenMoveComponent::Update(float delta)
{
	POINT mouse = Input::GetMousePosition();

	Vector2 mousePos =
	{
		static_cast<float>(mouse.x),
		static_cast<float>(mouse.y)
	};

	float w = Application::GetWidth();
	float h = Application::GetHeight();

	Vector2 center = { w * 0.5f,h * 0.5f };

	Vector2 mouseDistance = mousePos - center;

	mouseDistance.x /= (w * 0.5f);
	mouseDistance.y /= (h * 0.5f);

	mouseDistance.y *= -1.0f;

	m_targetOffset = mouseDistance * m_range;

	m_targetOffset.x = std::clamp(m_targetOffset.x, -m_range, m_range);
	m_targetOffset.y = std::clamp(m_targetOffset.y, -m_range, m_range);

	Vector2 toTarget = m_targetOffset - m_currentOffset;

	m_velocity += toTarget * m_followPower * delta;
	m_velocity *= m_damping;

	m_currentOffset += m_velocity * delta;

	m_currentOffset.x = std::clamp(m_currentOffset.x, -m_range, m_range);
	m_currentOffset.y = std::clamp(m_currentOffset.y, -m_range, m_range);

	Vector3 railCenter = m_forwardComp->GetRailCenterPos();
	Vector3 railRight = m_forwardComp->GetRailRight();
	Vector3 railUp = m_forwardComp->GetRailUp();

	Vector3 finalPos =
		railCenter
		+ railRight * m_currentOffset.x
		+ railUp * m_currentOffset.y;

	GetOwner()->SetPosition(finalPos);
}

void ScreenMoveComponent::Uninit()
{

}