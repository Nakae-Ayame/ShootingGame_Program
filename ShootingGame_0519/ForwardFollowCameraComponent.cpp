#include "ForwardFollowCameraComponent.h"
#include "GameObject.h"
#include <iostream>

using namespace DirectX::SimpleMath;

void ForwardFollowCameraComponent::Initialize()
{

}

void ForwardFollowCameraComponent::Update(float dt)
{
    if (!m_forwardComp){ return; }

    Vector3 railCenter = m_forwardComp->GetRailCenterPos();
    Vector3 forward = m_forwardComp->GetRailForward();
    Vector3 up = m_forwardComp->GetRailUp();
    Vector3 right = m_forwardComp->GetRailRight();

    // ”O‚Ì‚½‚ß³‹K‰»
    if (forward.LengthSquared() > 0.000001f)
    {
        forward.Normalize();
    }
    if (right.LengthSquared() > 0.000001f)
    {
        right.Normalize();
    }
    if (up.LengthSquared() > 0.000001f)
    {
        up.Normalize();
    }

    Vector3 desiredPos =
        railCenter
        - forward * m_backDistance
        + up * m_height
        + right * m_sideOffset;

    m_position = desiredPos;
    m_forward = forward;
    m_right = right;

    m_aimPoint = railCenter + forward * m_lookAhead;

    m_ViewMatrix = Matrix::CreateLookAt(m_position, m_aimPoint, up);

    UpdateProjectionIfNeeded();
	std::cout << "Camera Position: " << m_position.x << ", " << m_position.y << ", " << m_position.z << std::endl;
}

void ForwardFollowCameraComponent::Draw(float dt)
{
}

Vector3 ForwardFollowCameraComponent::GetAimDirectionFromReticle() const
{
    Vector3 dir = (m_aimPoint - m_position);
    if (dir.LengthSquared() <= 0.000001f)
    {
        return Vector3::Forward;
    }

    dir.Normalize();
    return dir;
}