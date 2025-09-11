#pragma once
#include "GameObject.h"
#include "BulletComponent.h"
#include "SphereComponent.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class OBBColliderComponent; // forward

class Bullet : public GameObject
{
public:
    Bullet() {};
    ~Bullet() override = default;

    void Initialize() override;
    void Update(float dt) override; // もし基底が Update() ならシグネチャ合わせてください
    void Draw(float alpha) override;

    // 見た目（今は省略orデバッグ用）や衝突サイズ設定
    void SetRadius(float r) { m_radius = r; }
    //void SetLifeTime(float s){};

private:
    float m_radius = 1.0f;
    std::shared_ptr<OBBColliderComponent> m_collider;
    std::shared_ptr<SphereComponent> m_bullet;
};


