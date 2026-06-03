#pragma once
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

class SpringVector3
{
public:
    SpringVector3() = default;

    //ばねのリセット
    void Reset(const Vector3& pos)
    {
        m_position = pos;
        m_velocity = Vector3::Zero;
    }

    void Update(const Vector3& target, float deltaTime)
    {
        if (deltaTime <= 0.0f) return;

        Vector3 displacement = m_position - target;
        Vector3 springForce = -m_stiffness * displacement;
        Vector3 dampingForce = -m_damping * m_velocity;

        Vector3 force = springForce + dampingForce;
        Vector3 acceleration = force / m_mass;

        m_velocity += acceleration * deltaTime;
        m_position += m_velocity * deltaTime;
    }

    //剛性(ばねの固さ)のセット関数
    void SetStiffness(float k)
    {
        m_stiffness = k;
    }

    //減衰(ばねの)のセット関数
    void SetDamping(float d)
    {
        m_damping = d;
    }

    //質量(追尾の反応の速さなどのため)のセット関数
    void SetMass(float m)
    {
        m_mass = m;
    }

    //今現在のばねの結果とした位置
    Vector3 GetPosition() const { return m_position; }

private:
    //今の位置を保存する変数
    Vector3 m_position = Vector3::Zero;

    //今のベクトルを保存する変数
    Vector3 m_velocity = Vector3::Zero;

    //剛性
    float m_stiffness = 10.0f;
    //減衰
    float m_damping = 5.0f;
    //質量
    float m_mass = 1.0f;
};

