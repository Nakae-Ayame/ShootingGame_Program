#include "SphereComponent.h"
#include "Renderer.h"
#include "GameObject.h"
#include <cassert>

using namespace DirectX::SimpleMath;

// 共有ユニット球
static std::shared_ptr<Primitive> s_sharedSphere = nullptr;

void SphereComponent::Initialize()
{
    ID3D11Device* device = Renderer::GetDevice();
    assert(device && "Renderer::GetDevice() is null in SphereComponent::Initialize");

    if (!s_sharedSphere)
    {
        s_sharedSphere = std::make_shared<Primitive>();
        // ユニット球（radius = 1）。GameObject のスケールで実際の半径を表現する。
        s_sharedSphere->CreateSphere(device, 1.0f, m_sliceCount, m_stackCount);
    }
}

void SphereComponent::Draw(float /*alpha*/)
{
    if (!GetOwner())
    {
        return;
    }

    Renderer::SetDepthEnable(false);

    Matrix4x4 world = GetOwner()->GetTransform().GetMatrix();
    Renderer::SetWorldMatrix(&world);

    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx){ return; }

    if (s_sharedSphere)
    {
        s_sharedSphere->Draw(ctx);
    }
}
