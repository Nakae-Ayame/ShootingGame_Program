#pragma once
#include "GameObject.h"
#include "ModelComponent.h"
#include "MoveComponent.h"
#include "ShootingComponent.h"
#include "AABBColliderComponent.h"
#include "OBBColliderComponent.h"

class Player : public GameObject
{
public:
    Player() = default;
    ~Player() override = default;

    void Initialize() override;     //‰Šú‰»
    void Update(float dt) override; //XV

    //Õ“Ë”»’èŠÖ”
    void OnCollision(GameObject* other) override;
private:
    std::shared_ptr<OBBColliderComponent> m_Collider;
};

