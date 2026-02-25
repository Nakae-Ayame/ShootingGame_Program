#pragma once
#include "GameObject.h"

class OBBColliderComponent;

class RailShooterPlayer : public GameObject
{
public:
    RailShooterPlayer() = default;
    ~RailShooterPlayer() override = default;

    void Initialize() override;
    void Update(float dt) override;

    void OnCollision(GameObject* other) override;

private:
    std::shared_ptr<OBBColliderComponent> m_Collider;
};
