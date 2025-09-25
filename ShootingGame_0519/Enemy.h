#pragma once
#include "GameObject.h"
#include "ModelComponent.h"
#include "MoveComponent.h"
#include "ShootingComponent.h"
#include "AABBColliderComponent.h"
#include "OBBColliderComponent.h"

class Enemy : public GameObject
{
public:
    Enemy() = default;
    ~Enemy() override = default;

    void Initialize() override;         //‰Šú‰»
    void Update(float dt) override;     //XV
    void OnCollision(GameObject* other) override;     //“–‚½‚Á‚½‚Ìˆ—
private:
    std::shared_ptr<OBBColliderComponent> m_Collider;
};

