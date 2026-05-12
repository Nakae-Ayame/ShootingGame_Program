#pragma once
#include "Component.h"
#include <SimpleMath.h>
#include <functional>

using namespace DirectX::SimpleMath;

class BulletComponent : public Component
{
public:
    enum BulletType
    {
        UNKNOW,
        PLAYER,
        ENEMY
    };

    BulletComponent() = default;
    ~BulletComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;

    //-------------Set関数--------------
    void SetVelocity(const Vector3& velocity);
    void SetSpeed(float speed) { m_speed = speed; }
    void SetLifetime(float lifetime) { m_lifetime = lifetime; }
    void SetBulletType(BulletType type) { m_ownerType = type; }
    void SetColor(const Vector4& color) { m_color = color; }

    void SetFollowAim(bool followAim) { m_followAim = followAim; }
    void SetTurnSpeed(float turnSpeed) { m_turnSpeed = turnSpeed; }
    void SetAimTargetProvider(const std::function<Vector3()>& provider) { m_aimTargetProvider = provider; }

    //-------------Get関数--------------
    Vector3 GetVelocity() const { return m_velocity; }
    float GetSpeed() const { return m_speed; }
    float GetLifetime() const { return m_lifetime; }
    BulletType GetBulletType() const { return m_ownerType; }
    Vector4 GetColor() const { return m_color; }
    bool GetFollowAim() const { return m_followAim; }

private:
    //--------------移動関連------------------
    Vector3 m_velocity = Vector3::Forward;
    float m_speed = 40.0f;

    //--------------寿命関連------------------
    float m_age = 0.0f;
    float m_lifetime = 1.0f;

    //--------------照準追従関連------------------
    bool m_followAim = false;
    float m_turnSpeed = 12.0f;
    std::function<Vector3()> m_aimTargetProvider;

    //--------------メタ情報関連------------------
    BulletType m_ownerType = BulletType::UNKNOW;

    //--------------見た目関連------------------
    Vector4 m_color = Vector4(1, 1, 1, 1);
};