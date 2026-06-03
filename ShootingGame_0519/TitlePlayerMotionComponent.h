#pragma once
#include "Component.h"
#include <SimpleMath.h>
#include <algorithm>

struct BezierPath
{
    DirectX::SimpleMath::Vector3 p0;
    DirectX::SimpleMath::Vector3 p1;
    DirectX::SimpleMath::Vector3 p2;
    DirectX::SimpleMath::Vector3 p3;
    float duration = 2.0f;
};

class TitlePlayerMotionComponent : public Component
{
public:
	TitlePlayerMotionComponent() = default;
	~TitlePlayerMotionComponent() = default;
	void Initialize();
	void Update(float dt);

    //--------Set関数-------
    void SetBasePosition(const DirectX::SimpleMath::Vector3& pos) { m_basePos = pos; }
    void SetRotateSpeed(float radPerSec) { m_rotateSpeed = radPerSec; }
    void SetBobAmplitude(float amp) { m_bobAmplitude = amp; }
	void SetDuration(float duration) { m_duration = duration; }
    void SetBobSpeed(float speed) { m_bobSpeed = speed; }
    void SetControlPoints(const DirectX::SimpleMath::Vector3& p0,
                          const DirectX::SimpleMath::Vector3& p1,
                          const DirectX::SimpleMath::Vector3& p2,
                          const DirectX::SimpleMath::Vector3& p3);
    void SetLogoTriggerT(float triggerT) { m_logoTriggerT = std::clamp(triggerT, 0.0f, 1.0f); }
    void SetOnLogoTrigger(std::function<void()> onTrigger) { m_onLogoTrigger = onTrigger; }
    void ResetTime() { m_time = 0.0f; }
    void SetModelYawOffset(float rad) { m_modelYawOffset = rad; }
    void SetModelPitchOffset(float rad) { m_modelPitchOffset = rad; }
    void SetModelRollOffset(float rad) { m_modelRollOffset = rad; }



    //--------Get関数-------
    float GetTime() const { return m_time; }
    float GetDuration() const { return m_duration; }
    float GetNormalizedTime() const
    {
        return std::clamp(m_time / max(0.01f, m_duration), 0.0f, 1.0f);
    }
    bool IsFinished() const { return GetNormalizedTime() >= 1.0f; }
    float GetModelYawOffset() const { return m_modelYawOffset; }
    float GetModelPitchOffset() const { return m_modelPitchOffset; }
    float GetModelRollOffset() const { return m_modelRollOffset; }
private:
    //-------演出関連----------
    float m_time = 0.0f;
	float m_duration = 1.5f; //移動にかかる時間
    float m_triggerT = 0.6f;
    float m_logoTriggerT = 0.55f;
    bool m_hasTriggered = false;
    std::function<void()> m_onLogoTrigger;
    DirectX::SimpleMath::Vector3 m_basePos = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 10.0f);

    //--------------向き補正関連------------------
    float m_modelYawOffset   = 0.0f;    //
    float m_modelPitchOffset = 0.0f;    //
    float m_modelRollOffset  = 0.0f;    //

	//-----三次ベジェ曲線の制御点-----
	DirectX::SimpleMath::Vector3 m_p01 = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 m_p02 = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 m_p03 = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 m_p04 = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    DirectX::SimpleMath::Vector3 EvaluateBezier(float t) const;
    //DirectX::SimpleMath::Vector3 EvaluateBezierTangent(float t) const;

    float m_rotateSpeed = 0.6f;
    float m_bobAmplitude = 0.5f;
    float m_bobSpeed = 2.0f;
};
