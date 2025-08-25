#include "FollowCameraComponent.h"
#include "Renderer.h"
#include "Application.h"
#include "Input.h"

FollowCameraComponent::FollowCameraComponent()
{
    // スプリング設定
    m_Spring.SetStiffness(12.0f);
    m_Spring.SetDamping(6.0f);
    m_Spring.SetMass(1.0f);

    // プロジェクション行列
    float width = static_cast<float>(Application::GetWidth());
    float height = static_cast<float>(Application::GetHeight());
    m_ProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f), width / height, 0.1f, 1000.0f);
}

void FollowCameraComponent::SetTarget(GameObject* target)
{
    m_Target = target;
    if (target)
    {
        Vector3 initial = target->GetPosition() + Vector3(0, m_DefaultHeight, -m_DefaultDistance);
        m_Spring.Reset(initial);
    }
}

void FollowCameraComponent::Update()
{
    if (!m_Target) return;

    // エイムモード切替（右クリック）
    m_IsAiming = Input::IsMouseRightDown();

    // マウス移動で視点回転
    POINT delta = Input::GetMouseDelta();
    m_Yaw += delta.x * m_Sensitivity;
    m_Pitch += delta.y * m_Sensitivity;

    // 上下の回転角を制限
    m_Pitch = std::clamp(m_Pitch, m_PitchLimitMin, m_PitchLimitMax);

    // 左右も視界範囲を制限（±120度）
    m_Yaw = std::clamp(m_Yaw, -m_YawLimit, m_YawLimit);

    // カメラの目標位置を更新
    UpdateCameraPosition();

    // ビュー行列更新
    Vector3 cameraPos = m_Spring.GetPosition();
    Vector3 targetPos = m_Target->GetPosition();
    m_ViewMatrix = Matrix::CreateLookAt(cameraPos, targetPos, Vector3::Up);

    Renderer::SetViewMatrix(m_ViewMatrix);
    Renderer::SetProjectionMatrix(m_ProjectionMatrix);
}

void FollowCameraComponent::UpdateCameraPosition()
{
    float dist = m_IsAiming ? m_AimDistance : m_DefaultDistance;
    float height = m_IsAiming ? m_AimHeight : m_DefaultHeight;

    // 回転行列
    Matrix rotY = Matrix::CreateRotationY(m_Yaw);
    Matrix rotX = Matrix::CreateRotationX(m_Pitch);
    Matrix rotation = rotX * rotY;

    // 距離方向に回転オフセットを作成
    Vector3 offset = Vector3(0, 0, -dist);
    Vector3 rotatedOffset = Vector3::Transform(offset, rotation);

    Vector3 targetPos = m_Target->GetPosition();
    Vector3 desiredPos = targetPos + rotatedOffset + Vector3(0, height, 0);

    m_Spring.Update(desiredPos, 1.0f / 60.0f);
}

Vector3 FollowCameraComponent::GetForward() const
{
    return m_ViewMatrix.Invert().Forward();
}

Vector3 FollowCameraComponent::GetRight() const
{
    return m_ViewMatrix.Invert().Right();
}
