#pragma once
#include "Component.h"
#include "commontypes.h"
#include "GameObject.h"
#include "SpringVector3.h"
#include "ICameraViewProvider.h"
#include <DirectXMath.h>
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class FollowCameraComponent : public Component, public ICameraViewProvider
{
public:
    FollowCameraComponent();
    void Update() override;

    void SetTarget(GameObject* target);

    void SetDistance(float dist) { m_DefaultDistance = dist; }
    void SetHeight(float h) { m_DefaultHeight = h; }

    void SetSensitivity(float s) { m_Sensitivity = s; }
    float GetSensitivity() const { return m_Sensitivity; }

    Matrix GetView() const { return m_ViewMatrix; }
    Matrix GetProj() const { return m_ProjectionMatrix; }

    Vector3 GetForward() const override;
    Vector3 GetRight() const override;

private:
    void UpdateCameraPosition();

    GameObject* m_Target = nullptr;

    float m_DefaultDistance = 30.0f;
    float m_DefaultHeight = 2.5f;

    float m_AimDistance = 20.0f;
    float m_AimHeight = 1.8f;

    bool m_IsAiming = false;

    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;
    float m_Sensitivity = 0.01f;

    float m_PitchLimitMin = XMConvertToRadians(-15.0f);
    float m_PitchLimitMax = XMConvertToRadians(45.0f);
    float m_YawLimit = XMConvertToRadians(120.0f);

    Matrix m_ViewMatrix;
    Matrix m_ProjectionMatrix;

    SpringVector3 m_Spring;
};
