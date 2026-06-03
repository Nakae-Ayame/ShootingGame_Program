#define NOMINMAX
#include "FreeCameraComponent.h"
#include "Application.h"
#include "Input.h"
#include "Renderer.h"
#include <algorithm>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Vector3 FreeCameraComponent::GetForward() const
{
    return m_viewMatrix.Invert().Forward();
}

Vector3 FreeCameraComponent::GetRight() const
{
    return m_viewMatrix.Invert().Right();
}

Vector3 FreeCameraComponent::GetAimPoint() const
{
    return m_position + GetForward() * m_aimDistance;
}

Vector3 FreeCameraComponent::GetAimDirectionFromReticle() const
{
    return GetForward();
}

Vector3 FreeCameraComponent::GetUp() const
{
    return m_viewMatrix.Invert().Up();
}

Vector3 FreeCameraComponent::GetShootRayOrigin() const
{
    return GetPosition();
}

Vector3 FreeCameraComponent::GetShootRayDir() const
{
    return GetForward();
}

Vector2 FreeCameraComponent::GetReticleScreen() const
{
    float screenW = static_cast<float>(Application::GetWidth());
    float screenH = static_cast<float>(Application::GetHeight());
    return Vector2(screenW * 0.5f, screenH * 0.5f);
}

void FreeCameraComponent::Initialize()
{
    UpdateProjectionIfNeeded();
    m_position = Vector3(0.0f, 0.0f, 0.0f);
    m_viewMatrix = Matrix::CreateLookAt(
        m_position,
        m_position + Vector3::Forward,
        Vector3::Up
    );
}

void FreeCameraComponent::Update(float dt)
{
    if (dt <= 0.0f)
    {
        return;
    }

    UpdateProjectionIfNeeded();

    Vector3 forward = Vector3::Forward;
    Vector3 right = Vector3::Right;

    float speed = m_moveSpeed;
    if (Input::IsKeyDown(VK_SHIFT))
    {
        speed = m_boostSpeed;
    }

    Vector3 move = Vector3::Zero;

    if (move.LengthSquared() > 1e-6f)
    {
        move.Normalize();
        m_position += move * speed * dt;
    }

    m_viewMatrix = Matrix::CreateLookAt(
        m_position,
        m_position + Vector3::Forward,
        Vector3::Up
    );


    Renderer::SetViewMatrix(m_viewMatrix);
    Renderer::SetProjectionMatrix(m_projectionMatrix);
}

void FreeCameraComponent::UpdateViewMatrix()
{
    Matrix rot = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    Vector3 forward = Vector3::TransformNormal(Vector3::Forward, rot);
    if (forward.LengthSquared() > 1e-6f)
    {
        forward.Normalize();
    }

    Vector3 lookAt = m_position + forward;
    m_viewMatrix = Matrix::CreateLookAt(m_position, lookAt, Vector3::Up);
}
