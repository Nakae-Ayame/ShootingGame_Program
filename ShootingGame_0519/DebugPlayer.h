#pragma once
#include "GameObject.h"
#include "ModelComponent.h"
#include "DebugMoveComponent.h"
#include "BoxComponent.h"
#include "AABBColliderComponent.h"
#include "OBBColliderComponent.h"

class DebugPlayer : public GameObject
{
public:
    DebugPlayer() = default;
    ~DebugPlayer() = default;

    void Initialize() override;
    void Update(float dt) override;
    void Draw(float dt) override;

private:
};
