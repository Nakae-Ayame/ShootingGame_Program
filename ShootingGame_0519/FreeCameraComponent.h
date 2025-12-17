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
    DirectX::SimpleMath::Matrix  GetView() const override { return m_ViewMatrix; }
    DirectX::SimpleMath::Matrix  GetProj() const override { return m_ProjectionMatrix; }
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_Position; }
    DirectX::SimpleMath::Vector3 GetAimDirectionFromReticle() const override;
    DirectX::SimpleMath::Vector2 GetReticleScreen() const override;

    //-----------------------------------Setä÷êîä÷òA------------------------------------
    void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
    void SetBoostSpeed(float speed) { m_BoostSpeed = speed; }
    void SetMouseSensitivity(float s) { m_MouseSensitivity = s; }

private:
    void UpdateViewMatrix();

    DirectX::SimpleMath::Vector3 m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    float m_MoveSpeed = 30.0f;
    float m_BoostSpeed = 18.0f;
    float m_MouseSensitivity = 0.0015f;

    float m_PitchLimitMin = DirectX::XMConvertToRadians(-80.0f);
    float m_PitchLimitMax = DirectX::XMConvertToRadians(80.0f);

    float m_AimDistance = 300.0f;
};

