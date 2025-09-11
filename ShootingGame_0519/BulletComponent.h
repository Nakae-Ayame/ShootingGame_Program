#pragma once
#include "Component.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class BulletComponent : public Component
{
public:
    BulletComponent() = default;
    ~BulletComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;

    //外部(主にShootingComponentからかな)から決めるためのセッター関数
    //方向ベクトル
    void SetVelocity(const Vector3& v) { m_velocity = v; }

    //スピード
    void SetSpeed(float s) { m_speed = s; }

    //弾の生存時間
    void SetLifetime(float sec) { m_lifetime = sec; }

private:
    Vector3 m_velocity = Vector3::Zero; //方向ベクトル(単位ベクトルが望ましい)
    float m_speed = 40.0f;              //スピード
    float m_age = 0.0f;                 //経過時間
    float m_lifetime = 5.0f;            //生存時間
};

