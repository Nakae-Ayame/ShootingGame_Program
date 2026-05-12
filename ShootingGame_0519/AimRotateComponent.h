#pragma once
#include "Component.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class ScreenMoveComponent;
class ForwardMoveComponent;

class AimRotateComponent : public Component
{
public:
    AimRotateComponent() = default;
    ~AimRotateComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;
    void Uninit() override;

    //--------Set関数-------
    void SetScreenMoveComponent(ScreenMoveComponent* screenMoveComp) { m_screenMoveComp = screenMoveComp; }
    void SetRailProvider(ForwardMoveComponent* forwardComp) { m_forwardComp = forwardComp; }

    void SetMaxYawAngle(float angle) { m_maxYawAngle = angle; }
    void SetMaxPitchAngle(float angle) { m_maxPitchAngle = angle; }
    void SetMaxRollAngle(float angle) { m_maxRollAngle = angle; }
    void SetTiltVelocityRange(float range) { m_tiltVelocityRange = range; }
    void SetRotateLerpSpeed(float speed) { m_rotateLerpSpeed = speed; }
    void SetBaseRotation(const Vector3& baseRotation) { m_baseRotation = baseRotation; }

    //--------Get関数-------
    float GetCurrentYaw() const { return m_currentYaw; }
    float GetCurrentPitch() const { return m_currentPitch; }
    float GetCurrentRoll() const { return m_currentRoll; }

private:
    float Clamp(float value, float minValue, float maxValue);

private:
    //--------------参照関連------------------
    ScreenMoveComponent* m_screenMoveComp = nullptr;
    ForwardMoveComponent* m_forwardComp = nullptr;

    //--------------角度設定関連------------------
    float m_maxYawAngle = 12.0f;
    float m_maxPitchAngle = 8.0f;
    float m_maxRollAngle = 18.0f;
    float m_tiltVelocityRange = 30.0f;
    float m_rotateLerpSpeed = 8.0f;

    //--------------現在角度関連------------------
    float m_currentYaw = 0.0f;
    float m_currentPitch = 0.0f;
    float m_currentRoll = 0.0f;

    //--------------モデル補正関連------------------
    Vector3 m_baseRotation = Vector3::Zero;
};