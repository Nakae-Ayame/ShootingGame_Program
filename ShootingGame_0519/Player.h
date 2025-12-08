#pragma once
#include "GameObject.h"
#include "OBBColliderComponent.h"
#include "AABBColliderComponent.h"

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

    // penetration ‰ğÁ—p
    //void ResolvePenetrationWith(GameObject* other);
    //bool IsCollidingWith(GameObject* other) const;
};

