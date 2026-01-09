#pragma once
#include "Component.h"
#include <SimpleMath.h>
#include <memory>

using namespace DirectX::SimpleMath;

class GameObject;
class BulletComponent;

class HomingComponent : public Component
{
public:
    HomingComponent() = default;
    ~HomingComponent() override = default;

    //--------Set/Get関数-------
    void SetTarget(const std::weak_ptr<GameObject>& t) { m_target = t; }
    std::weak_ptr<GameObject> GetTarget() const { return m_target; }

    // timeToIntercept: 「あと何秒で命中させるか」。大きいほど緩やか。
    void SetTimeToIntercept(float t) { m_timeToIntercept = t; }
    float GetTimeToIntercept() const { return m_timeToIntercept; }

    // 加速度上限（0 なら無制限）
    void SetMaxAcceleration(float a) { m_maxAcceleration = a; }
    float GetMaxAcceleration() const { return m_maxAcceleration; }

    // ライフタイム（ホーミングを作った弾に適用する場合）
    void SetLifeTime(float sec) { m_lifeTime = sec; }

    //--------その他関数-------
    void Initialize() override;
    void Update(float dt) override;

private:
    //--------------追尾関連------------------
    std::weak_ptr<GameObject> m_target;
    float m_timeToIntercept = 1.5f;   // デフォルト 1 秒で命中を目指す
    float m_maxAcceleration = 400.0f;   // 0 = 無制限、>0 で制限
    float m_lifeTime = 5.0f;          // Homing の寿命（optional）
    float m_age = 0.0f;               // 経過時間

    Vector3 m_prevTargetPos = Vector3::Zero;
    bool m_havePrevTargetPos = false;
    float m_maxTurnRateDeg = 60.0f;
};