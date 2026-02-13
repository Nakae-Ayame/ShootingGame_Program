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
    virtual void SetFov(float fovRad) { m_Fov = fovRad; }
    void SetNearFar(float nearZ, float farZ) { m_NearZ = nearZ; m_FarZ = farZ; }

    //-----------------------------------Getä÷êîä÷òA------------------------------------
    DirectX::SimpleMath::Matrix GetView() const { return m_ViewMatrix; }
    DirectX::SimpleMath::Matrix GetProj() const { return m_ProjectionMatrix; }
    float GetFov() const { return m_Fov; }
    float GetNearZ() const { return m_NearZ; }
    float GetFarZ() const { return m_FarZ; }

protected:
    void UpdateProjectionIfNeeded();

    DirectX::SimpleMath::Matrix m_ViewMatrix{};
    DirectX::SimpleMath::Matrix m_ProjectionMatrix{};

    float m_Fov = DirectX::XMConvertToRadians(45.0f);
    float m_NearZ = 0.1f;
    float m_FarZ = 1000.0f;

    int m_PrevScreenW = 0;
    int m_PrevScreenH = 0;
};

