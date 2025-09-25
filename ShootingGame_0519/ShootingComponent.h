#pragma once
#include "Component.h"
#include "IScene.h"
#include "ICameraViewProvider.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Bullet; // 前方宣言

class ShootingComponent : public Component
{
public:
    ShootingComponent() = default;
    ~ShootingComponent() override = default;

    void Initialize() override {}
    // dt は秒（例: 1/60.0f）
    void Update(float dt) override;

    // シーンとカメラを外部からセット
    void SetScene(IScene* scene) { m_scene = scene; }
    void SetCamera(ICameraViewProvider* camera) { m_camera = camera; }

    // 設定
    void SetCooldown(float cd) { m_cooldown = cd; }
    void SetBulletSpeed(float sp) { m_bulletSpeed = sp; }
    void SetSpawnOffset(float off) { m_spawnOffset = off; }

private:
    // 弾生成のヘルパー（GameObjectを返します）
    std::shared_ptr<GameObject> CreateBullet(const Vector3& pos, const Vector3& dir);

    IScene* m_scene = nullptr;
    ICameraViewProvider* m_camera = nullptr;

    float m_cooldown = 0.25f;   // 発射間隔（秒）
    float m_timer = 0.0f;
    float m_bulletSpeed = 180.0f;
    float m_spawnOffset = 1.5f; // プレイヤー前方に出現させるオフセット
};


