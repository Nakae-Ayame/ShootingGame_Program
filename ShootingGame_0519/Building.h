#pragma once
#include "GameObject.h"
#include "ModelComponent.h"
#include "MoveComponent.h"
#include "ShootingComponent.h"
#include "AABBColliderComponent.h"
#include "OBBColliderComponent.h"

class Building : public GameObject
{
public:
    Building() = default;
    ~Building() override = default;

    void Initialize() override;     //‰Šú‰»
    void Update(float dt) override; //XV

private:

};

