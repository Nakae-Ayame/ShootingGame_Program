#include "FloorComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "TextureManager.h"
#include "DebugRenderer.h"
#include <vector>

void FloorComponent::Initialize()
{
    // テクスチャが指定されているならロード
    if (m_mode == GridMode::Textured &&!m_texture.empty())
    {
        ID3D11ShaderResourceView* srv = TextureManager::Load(m_texture);
        if (srv) { m_gridSRV = srv; }
    }

    auto device = Renderer::GetDevice();

    if (!device) 
    {
        OutputDebugStringA("FloorComponent::Initialize - Renderer::GetDevice() == nullptr\n");
        return;
    }

    m_prim.CreatePlane(device, m_width, m_depth, m_tileU, m_tileV);
}

void FloorComponent::Draw(float alpha)
{
    GameObject* owner = GetOwner();
    if (!owner) return;

    // 正しい変換でワールド行列をセット
    DirectX::XMMATRIX xmWorld = owner->GetWorldMatrix(); // SimpleMath::Matrix -> XMMATRIX へ
    DirectX::XMFLOAT4X4 worldF;
    DirectX::XMStoreFloat4x4(&worldF, xmWorld);
    Renderer::SetWorldMatrix(reinterpret_cast<Matrix4x4*>(&worldF)); // Matrix4x4 が XMFLOAT4X4 ならOK

    if (m_mode == GridMode::Lines)
    {
        DebugRenderer& dr = DebugRenderer::Get();
        Vector3 pos = owner->GetPosition();
        float y = pos.y + m_y;

        float startX = -m_width * 0.5f;
        float endX = m_width * 0.5f;
        float startZ = -m_depth * 0.5f;
        float endZ = m_depth * 0.5f;

        for (float x = startX; x <= endX + 1e-6f; x += m_gridStep)
            dr.AddLine(Vector3(x, y, startZ), Vector3(x, y, endZ), m_color);

        for (float z = startZ; z <= endZ + 1e-6f; z += m_gridStep)
            dr.AddLine(Vector3(startX, y, z), Vector3(endX, y, z), m_color);

        return;
    }

    //マテリアル設定
    MATERIAL mat{};
    mat.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
    if (m_gridSRV) mat.TextureEnable = TRUE;
    else mat.TextureEnable = FALSE;
    Renderer::SetMaterial(mat);

    //テクスチャバインド
    if (m_gridSRV)
    {
        Renderer::SetTexture(m_gridSRV);
    }
    else
    {
        //なければnullをセット
        Renderer::SetTexture(nullptr);
    }

    //Primitiveを描画
    m_prim.Draw(Renderer::GetDeviceContext());

    //PSのSRVを解除しておくと安全
    ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, nullSrv);
}
