#pragma once
#include "GameObject.h"
#include "BulletComponent.h"
#include "Primitive.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class OBBColliderComponent; //‘O•ûéŒ¾

class Bullet : public GameObject
{
public:
    Bullet() {};
    ~Bullet() override = default;

    void Initialize() override;
    void Update(float dt) override;
    void Draw(float alpha) override;
    void OnCollision(GameObject* other) override;

    //’e‚Ì”­ËÒ‚ÌType
   /* enum BulletType
    {
        Player,
        Enemy
    };*/

    void SetRadius(float r) { m_radius = r; }

private:
    float m_radius = 1.0f;

    Primitive m_primitive;

    std::shared_ptr<OBBColliderComponent> m_collider;
};


