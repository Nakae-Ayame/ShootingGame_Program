#include "AimRotateComponent.h"
#include "ScreenMoveComponent.h"
#include "ForwardMoveComponent.h"
#include "GameObject.h"
#include <algorithm>

float AimRotateComponent::Clamp(float value, float minValue, float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }

    if (value > maxValue)
    {
        return maxValue;
    }

    return value;
}

void AimRotateComponent::Initialize()
{

}

void AimRotateComponent::Update(float dt)
{
    if (!GetOwner()){ return; }

    if (!m_screenMoveComp){ return; }

    Vector2 currentOffset = m_screenMoveComp->GetCurrentOffset();
    Vector2 targetOffset = m_screenMoveComp->GetTargetOffset();

    Vector2 toTarget = targetOffset - currentOffset;

    float normalizedX = 0.0f;
    float normalizedY = 0.0f;

    if (m_tiltVelocityRange > 0.0001f)
    {
        normalizedX = Clamp(toTarget.x / m_tiltVelocityRange, -1.0f, 1.0f);
        normalizedY = Clamp(toTarget.y / m_tiltVelocityRange, -1.0f, 1.0f);
    }

    float targetYaw   = -normalizedX * m_maxYawAngle;
    float targetPitch = -normalizedY * m_maxPitchAngle;

    float targetRoll = 0.0f;

    if (toTarget.LengthSquared() < 0.05f)
    {
        targetYaw = 0.0f;
        targetPitch = 0.0f;
        targetRoll = 0.0f;
    }

    float lerpFactor = m_rotateLerpSpeed * dt;
    lerpFactor = Clamp(lerpFactor, 0.0f, 1.0f);

    m_currentYaw += (targetYaw - m_currentYaw) * lerpFactor;
    m_currentPitch += (targetPitch - m_currentPitch) * lerpFactor;
    m_currentRoll += (targetRoll - m_currentRoll) * lerpFactor;

    Vector3 finalRotation =
    {
        m_baseRotation.x + m_currentPitch,
        m_baseRotation.y + m_currentYaw,
        m_baseRotation.z + m_currentRoll
    };

    GetOwner()->SetRotation(finalRotation);
}

void AimRotateComponent::Uninit()
{

}