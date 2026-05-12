#include "ShootingComponent.h"
#include "Bullet.h"
#include "BulletComponent.h"
#include "Input.h"
#include "GameObject.h"
#include "IScene.h"
#include "Sound.h"
#include "RaycastHit.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

void ShootingComponent::Update(float dt)
{
    m_timer += dt;

    GameObject* owner = GetOwner();

    if (!owner)
    {
        return;
    }

    UpdateAimInfo(owner);

    bool wantFire = m_autoFire || Input::IsKeyDown(VK_SPACE);

    if (!wantFire)
    {
        return;
    }

    if (m_timer < m_cooldown)
    {
        return;
    }

    Fire();
}

void ShootingComponent::UpdateAimInfo(GameObject* owner)
{
    if (!owner)
    {
        return;
    }

    Vector3 rayOrigin = owner->GetPosition();
    Vector3 rayDir = owner->GetForward();

    if (m_camera)
    {
        rayOrigin = m_camera->GetShootRayOrigin();
        rayDir = m_camera->GetShootRayDir();
    }

    if (rayDir.LengthSquared() <= 1e-6f)
    {
        rayDir = owner->GetForward();
    }

    if (rayDir.LengthSquared() <= 1e-6f)
    {
        rayDir = Vector3::Forward;
    }

    rayDir.Normalize();

    const float maxDistance = 3000.0f;

    Vector3 aimPoint = rayOrigin + rayDir * maxDistance;

    IScene* scene = m_scene;

    if (!scene)
    {
        scene = owner->GetScene();
    }

    if (scene)
    {
        RaycastHit hit{};

        bool hitSomething = scene->Raycast(
            rayOrigin,
            rayDir,
            maxDistance,
            hit,
            [owner](GameObject* obj)
            {
                if (!obj)
                {
                    return false;
                }

                if (obj == owner)
                {
                    return false;
                }

                return true;
            },
            owner);

        if (hitSomething)
        {
            aimPoint = hit.point;
        }
    }

    m_currentAimDirection = rayDir;
    m_currentAimPoint = aimPoint;
}

std::shared_ptr<GameObject> ShootingComponent::CreateBullet(
    const Vector3& position,
    const Vector3& direction,
    const Vector4& color)
{
    auto bullet = std::make_shared<Bullet>();
    bullet->Initialize();
    bullet->SetPosition(position);

    auto bulletComp = bullet->GetComponent<BulletComponent>();

    if (bulletComp)
    {
        Vector3 bulletDir = direction;

        if (bulletDir.LengthSquared() <= 1e-6f)
        {
            bulletDir = Vector3::Forward;
        }

        bulletDir.Normalize();

        bulletComp->SetVelocity(bulletDir);
        bulletComp->SetSpeed(m_bulletSpeed);
        bulletComp->SetBulletType(BulletComponent::PLAYER);
        bulletComp->SetColor(color);

        bulletComp->SetFollowAim(true);
        bulletComp->SetTurnSpeed(m_followAimTurnSpeed);

        bulletComp->SetAimTargetProvider([this]()
            {
                return m_currentAimPoint;
            });
    }

    return bullet;
}

void ShootingComponent::AddBulletToScene(const std::shared_ptr<GameObject>& bullet)
{
    if (!bullet)
    {
        return;
    }

    if (m_scene)
    {
        m_scene->AddObject(bullet);
        return;
    }

    GameObject* owner = GetOwner();

    if (!owner)
    {
        return;
    }

    IScene* scene = owner->GetScene();

    if (!scene)
    {
        return;
    }

    scene->AddObject(bullet);
}

void ShootingComponent::Fire()
{
    GameObject* owner = GetOwner();
    if (!owner) { return; }
    if (m_timer < m_cooldown) { return; }

    UpdateAimInfo(owner);

    // ★ ポイント1: Player の現在の前方ベクトルを正しく取得
    Vector3 forward = owner->GetForward();
    if (forward.LengthSquared() <= 1e-6f)
    {
        forward = Vector3::Forward;
    }
    forward.Normalize();

    // ★ ポイント2: Player の回転を考慮した生成位置
    Vector3 ownerPos = owner->GetPosition();
    Vector3 spawnPos = ownerPos + forward * m_spawnOffset;

    // ★ ポイント3: カメラ側の照準点から弾へのベクトルを計算
    Vector3 bulletDir = m_currentAimPoint - spawnPos;

    if (bulletDir.LengthSquared() <= 1e-6f)
    {
        bulletDir = m_currentAimDirection;
    }
    if (bulletDir.LengthSquared() <= 1e-6f)
    {
        return;
    }
    bulletDir.Normalize();

    auto bullet = CreateBullet(spawnPos, bulletDir, m_normalBulletColor);
    if (!bullet) { return; }

    AddBulletToScene(bullet);
    Sound::PlaySeWav(L"Asset/Sound/SE/PlayerShot_SE.wav", 0.3f);

    m_timer = 0.0f;
}