#pragma once
#include "GameObject.h"

class OBBColliderComponent;

class Player : public GameObject
{
public:
    Player() = default;
    ~Player() override = default;

    void Initialize() override;
    void Update(float dt) override;

    void OnCollision(GameObject* other) override;

private:
    std::shared_ptr<OBBColliderComponent> m_Collider;
};

