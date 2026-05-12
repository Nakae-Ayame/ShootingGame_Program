#pragma once
#include "Component.h"
#include "IScene.h"
#include "ICameraViewProvider.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class GameObject;
class BulletComponent;

class ShootingComponent : public Component
{
public:
    ShootingComponent() = default;
    ~ShootingComponent() override = default;

    void Update(float dt) override;

    //-------------Set関数--------------
    void SetScene(IScene* scene) { m_scene = scene; }
    void SetCamera(ICameraViewProvider* camera) { m_camera = camera; }

    void SetBulletSpeed(float speed) { m_bulletSpeed = speed; }
    void SetCooldown(float cooldown) { m_cooldown = cooldown; }
    void SetSpawnOffset(float offset) { m_spawnOffset = offset; }
    void SetAutoFire(bool autoFire) { m_autoFire = autoFire; }
    void SetNormalBulletColor(const Vector4& color) { m_normalBulletColor = color; }
    void SetFollowAimTurnSpeed(float turnSpeed) { m_followAimTurnSpeed = turnSpeed; }

    //-------------Get関数--------------
    Vector3 GetCurrentAimPoint() const { return m_currentAimPoint; }
    Vector3 GetCurrentAimDirection() const { return m_currentAimDirection; }

    //-------------その他関数--------------
    void Fire();

private:
    //--------------弾生成関連------------------
    std::shared_ptr<GameObject> CreateBullet(const Vector3& position, const Vector3& direction, const Vector4& color);
    void AddBulletToScene(const std::shared_ptr<GameObject>& bullet);

    //--------------照準関連------------------
    void UpdateAimInfo(GameObject* owner);

private:
    //--------------参照関連------------------
    IScene* m_scene = nullptr;
    ICameraViewProvider* m_camera = nullptr;

    //--------------発射設定関連------------------
    float m_cooldown = 0.1f;
    float m_timer = 0.0f;
    float m_bulletSpeed = 300.0f;
    float m_spawnOffset = 14.0f;
    bool m_autoFire = false;

    //--------------追従弾関連------------------
    float m_followAimTurnSpeed = 14.0f;

    //--------------照準情報関連------------------
    Vector3 m_currentAimPoint = Vector3::Forward * 3000.0f;
    Vector3 m_currentAimDirection = Vector3::Forward;

    //--------------見た目関連------------------
    Vector4 m_normalBulletColor = Vector4(1, 1, 1, 1);
};