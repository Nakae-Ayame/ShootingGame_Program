#pragma once
#include "Component.h"
#include "Primitive.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class SphereComponent : public Component
{
public:
    SphereComponent(float radius = 1.0f, int sliceCount = 16, int stackCount = 16)
        : m_radius(radius), m_sliceCount(sliceCount), m_stackCount(stackCount) {}

    ~SphereComponent() override = default;

    void Initialize() override;
    void Update(float dt) override {}
    void Draw(float alpha) override;

    void SetRadius(float r) { m_radius = r; } // •`‰æã‚Ì”¼ŒaiƒRƒ‰ƒCƒ_‚Æˆê’v‚³‚¹‚é“™j

private:
    Primitive m_primitive;
    float m_radius = 1.0f;
    int m_sliceCount = 16;
    int m_stackCount = 16;
};
