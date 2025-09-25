#include "SphereComponent.h"
#include "Renderer.h"
#include "GameObject.h"   
#include <cassert>

using namespace DirectX::SimpleMath;

void SphereComponent::Initialize()
{
    ID3D11Device* device = Renderer::GetDevice();
    assert(device && "Renderer::GetDevice() is null in SphereComponent::Initialize");

    // GPUバッファ作成（Primitive 内で頂点/インデックスを作る）
    m_primitive.CreateSphere(device, m_radius, m_sliceCount, m_stackCount);
}

void SphereComponent::Draw(float /*alpha*/)
{
    if (!GetOwner()) return;

    Renderer::SetDepthEnable(false);

    // ワールド行列を設定してから Primitive を描画
    Matrix4x4 world = GetOwner()->GetTransform().GetMatrix();
    Renderer::SetWorldMatrix(&world);

    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    if (!ctx) return;

    m_primitive.Draw(ctx);
}
