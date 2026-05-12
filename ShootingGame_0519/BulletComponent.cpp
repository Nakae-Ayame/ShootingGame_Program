#include "BulletComponent.h"
#include "GameObject.h"
#include "EffectManager.h"
#include "IScene.h"

using namespace DirectX::SimpleMath;

void BulletComponent::Initialize()
{
    if (m_velocity.LengthSquared() > 1e-6f)
    {
        m_velocity.Normalize();
    }
    else
    {
        m_velocity = Vector3::Forward;
    }

    if (m_speed < 0.0f)
    {
        m_speed = 20.0f;
    }

    m_age = 0.0f;
}

void BulletComponent::SetVelocity(const Vector3& velocity)
{
    m_velocity = velocity;

    if (m_velocity.LengthSquared() > 1e-6f)
    {
        m_velocity.Normalize();
    }
}

void BulletComponent::Update(float dt)
{
    GameObject* owner = GetOwner();
    if (!owner) { return; }

    m_age += dt;

    // ★ 寿命チェック
    if (m_age >= m_lifetime)
    {
        IScene* scene = owner->GetScene();
        if (scene) { scene->RemoveObject(owner); }
        return;
    }

    // ★ 照準追従（常時有効）
    if (m_followAim && m_aimTargetProvider)
    {
        Vector3 aimTarget = m_aimTargetProvider();
        Vector3 toTarget = aimTarget - owner->GetPosition();

        if (toTarget.LengthSquared() > 1e-6f)
        {
            toTarget.Normalize();

            Vector3 currentDir = m_velocity;
            if (currentDir.LengthSquared() <= 1e-6f)
            {
                currentDir = toTarget;
            }
            else
            {
                currentDir.Normalize();
            }

            // ★ ターン速度を高める（回転に追従しやすく）
            float lerpRate = m_turnSpeed * dt;
            if (lerpRate > 1.0f) { lerpRate = 1.0f; }

            currentDir = Vector3::Lerp(currentDir, toTarget, lerpRate);

            if (currentDir.LengthSquared() > 1e-6f)
            {
                currentDir.Normalize();
                m_velocity = currentDir;
            }
        }
    }

    // ★ 位置更新
    Vector3 prevPos = owner->GetPosition();
    Vector3 newPos = prevPos + m_velocity * m_speed * dt;
    owner->SetPosition(newPos);

    // ★ 軌跡エフェクト
    Vector3 moveVec = newPos - prevPos;
    if (moveVec.LengthSquared() > 1e-6f)
    {
        EffectManager::SpawnBulletTrail(prevPos, newPos);
    }
}