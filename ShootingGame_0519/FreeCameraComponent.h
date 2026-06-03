#pragma once
#include "CameraComponentBase.h"
#include <SimpleMath.h>

class FreeCameraComponent : public CameraComponentBase
{
public:
    FreeCameraComponent() = default;
    ~FreeCameraComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;

    //--------ICameraViewProvider é¿ëï--------
    DirectX::SimpleMath::Vector3 GetForward() const override;
    DirectX::SimpleMath::Vector3 GetRight() const override;
    DirectX::SimpleMath::Vector3 GetAimPoint() const override;
    DirectX::SimpleMath::Matrix  GetView() const override { return m_viewMatrix; }
    DirectX::SimpleMath::Matrix  GetProj() const override { return m_projectionMatrix; }
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }
    DirectX::SimpleMath::Vector3 GetAimDirectionFromReticle() const override;
    DirectX::SimpleMath::Vector2 GetReticleScreen() const override;

    DirectX::SimpleMath::Vector3 GetUp() const override;
    DirectX::SimpleMath::Vector3 GetShootRayOrigin() const override;
    DirectX::SimpleMath::Vector3 GetShootRayDir() const override;

    //-----------------------------------Setä÷êîä÷òA------------------------------------
    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetBoostSpeed(float speed) { m_boostSpeed = speed; }
    void SetMouseSensitivity(float s) { m_mouseSensitivity = s; }

private:
    void UpdateViewMatrix();

    DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    float m_moveSpeed = 30.0f;
    float m_boostSpeed = 18.0f;
    float m_mouseSensitivity = 0.0015f;

    float m_pitchLimitMin = DirectX::XMConvertToRadians(-80.0f);
    float m_pitchLimitMax = DirectX::XMConvertToRadians(80.0f);

    float m_aimDistance = 300.0f;
};

