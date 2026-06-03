#pragma once
#include "Component.h"
#include "ICameraViewProvider.h"
#include <SimpleMath.h>

class CameraComponentBase : public Component, public ICameraViewProvider
{
public:
    CameraComponentBase() = default;
    ~CameraComponentBase() override = default;

    void Initialize() override {}
    void Update(float dt) override = 0;

    //-----------------------------------Setä÷êîä÷òA------------------------------------
    virtual void SetFov(float fovRad) { m_fov = fovRad; }
    void SetNearFar(float nearZ, float farZ) { m_nearZ = nearZ; m_farZ = farZ; }

    //-----------------------------------Getä÷êîä÷òA------------------------------------
    DirectX::SimpleMath::Matrix GetView() const { return m_viewMatrix; }
    DirectX::SimpleMath::Matrix GetProj() const { return m_projectionMatrix; }
    float GetFov() const { return m_fov; }
    float GetNearZ() const { return m_nearZ; }
    float GetFarZ() const { return m_farZ; }

protected:
    void UpdateProjectionIfNeeded();

    DirectX::SimpleMath::Matrix m_viewMatrix{};
    DirectX::SimpleMath::Matrix m_projectionMatrix{};

    float m_fov = DirectX::XMConvertToRadians(45.0f);
    float m_nearZ = 0.1f;
    float m_farZ = 1000.0f;

    int m_prevScreenW = 0;
    int m_prevScreenH = 0;
};

