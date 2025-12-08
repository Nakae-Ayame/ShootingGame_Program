// MoveComponent.h
#pragma once
#include "Component.h"
#include "Input.h"
#include "ICameraViewProvider.h"
#include <SimpleMath.h>
#include <fstream>
#include <algorithm>
#include <functional>


class DebugMoveComponent : public Component
{
public:
    DebugMoveComponent() = default;
    ~DebugMoveComponent()  = default;

    void Initialize() override;
    void Update(float dt) override;
    void Uninit() override;

private:
    float m_moveSpeed = 20.0f; // ƒ†ƒjƒbƒg/•b
};

