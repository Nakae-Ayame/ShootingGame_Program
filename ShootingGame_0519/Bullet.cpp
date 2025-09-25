#include "Bullet.h"
#include "Enemy.h"
#include "OBBColliderComponent.h"
#include "CollisionManager.h"
#include "SphereComponent.h"
#include "Renderer.h" // optional: for debug draw
#include <iostream>

void Bullet::Initialize()
{
    //BulletComponentを追加して運動を担当させる
    auto m_bulletComp = std::make_shared<BulletComponent>();
    if (m_bulletComp)
    {
        m_bulletComp->SetLifetime(5.0f);
    }
    m_bulletComp->Initialize();
    AddComponent(m_bulletComp);

    //OBB コライダーを追加して衝突判定に参加させる（サイズは直径ベース）
    m_collider = AddComponent<OBBColliderComponent>();

    float radius = m_radius; // もし m_radius を外部から変えたいならここを書き換

    if (m_collider)
    {
        // OBBColliderComponent::SetSize() は全体サイズ（幅・高さ・奥行き）を期待する想定
        m_collider->SetSize(Vector3(m_radius * 2.0f, m_radius * 2.0f, m_radius * 2.0f));
    }

    ID3D11Device* dev = Renderer::GetDevice();
    m_primitive.CreateSphere(dev, m_radius, 16, 8); 
}

void Bullet::Update(float dt)
{   
    GameObject::Update(dt); 
}

void Bullet::Draw(float alpha)
{
    // world 行列セット
    Matrix4x4 world = GetTransform().GetMatrix();
    Renderer::SetWorldMatrix(&world);

    Renderer::SetDepthEnable(true);
    Renderer::DisableCulling(false);

    //保存: 現在 PS にバインドされている CB(slot3) を取得しておく ---
    ID3D11Buffer* prevPSCB = nullptr;
    Renderer::GetDeviceContext()->PSGetConstantBuffers(3, 1, &prevPSCB);

    // --- 2) マテリアルをセット（slot 3 を使う設計に合わせる） ---
    MATERIAL mat{};
    mat.Diffuse = Color(1.0f, 0.0f, 0.0f, 1.0f); // 赤
    Renderer::SetMaterial(mat);

    // --- 3) (オプションだが安全) 1x1 の赤テクスチャを作成して t0 にバインド ---
    // static にして一度だけ作る
    static ComPtr<ID3D11ShaderResourceView> s_redSRV;
    if (!s_redSRV)
    {
        ID3D11Device* dev = Renderer::GetDevice();
        ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();

        // 1x1 RGBA8 の赤ピクセル
        D3D11_TEXTURE2D_DESC td{};
        td.Width = 1; td.Height = 1; td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        uint32_t pixel = 0xFF0000FF; // BGRA or RGBA? R=255,G=0,B=0,A=255 -> DirectX expects little endian: 0xFF0000FF gives (B=0,R=255?) 
        // safer: use byte array
        uint8_t texData[4] = { 0x00, 0x00, 0xFF, 0xFF }; // B,G,R,A => (R=255) depending on format, but DXGI_FORMAT_R8G8B8A8_UNORM expects RGBA in memory.
        // So use RGBA: {255,0,0,255}
        uint8_t texRGBA[4] = { 255, 0, 0, 255 };

        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = texRGBA;
        sd.SysMemPitch = 4;

        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = dev->CreateTexture2D(&td, &sd, tex.GetAddressOf());
        if (SUCCEEDED(hr))
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
            srvd.Format = td.Format;
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels = 1;
            dev->CreateShaderResourceView(tex.Get(), &srvd, s_redSRV.GetAddressOf());
        }
        else
        {
            std::cout << "[Bullet] Failed to create 1x1 red texture\n";
        }
    }

    // 保存されている PS SRV を取り出して復元できるようにしておく
    ID3D11ShaderResourceView* prevSRV = nullptr;
    Renderer::GetDeviceContext()->PSGetShaderResources(0, 1, &prevSRV);

    // Bind red SRV to slot 0 (t0)
    if (s_redSRV) Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, s_redSRV.GetAddressOf());

    // --- 4) 描画 ---
    std::cout << "[Bullet] Drawing primitive (red)\n";
    m_primitive.Draw(Renderer::GetDeviceContext());

    // --- 5) 復元: PS SRV & PS CB を元に戻す ---
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &prevSRV);
    if (prevSRV) prevSRV->Release();

    Renderer::GetDeviceContext()->PSSetConstantBuffers(3, 1, &prevPSCB);
    if (prevPSCB) prevPSCB->Release();

    // 終了
}

void Bullet::OnCollision(GameObject* other)
{
    if (!other) return;

    // Enemy に衝突したら両方消す（他の弾やプレイヤーとは別扱いに）
    if (dynamic_cast<Enemy*>(other))
    {
        IScene* scene = GetScene();
        if (scene)
        {
            std::cout << "Enemyに衝突したためBulletを削除します " << std::endl;
            scene->RemoveObject(this);
            scene->RemoveObject(other);
        }
    }
}

