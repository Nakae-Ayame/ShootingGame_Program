#include "Bullet.h"
#include "OBBColliderComponent.h"
#include "CollisionManager.h"
#include "SphereComponent.h"
#include "Renderer.h" // optional: for debug draw
#include <iostream>

void Bullet::Initialize()
{
    OutputDebugStringA("Bullet::Initialize called\n");

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

        // CollisionManager に登録する（フレーム毎に登録/クリアする実装なら不要）
        CollisionManager::RegisterCollider(m_collider.get());
    }

    ID3D11Device* dev = Renderer::GetDevice();
    m_primitive.CreateSphere(dev, m_radius, 16, 8); 
}

void Bullet::Update(float dt)
{
    // 基底のコンポーネント Update を呼ぶ（GameObject::Update がコンポーネント群を回す実装をしている想定）
    GameObject::Update(dt); // もし GameObject::Update(float) に変えたならそちらを呼んでください

    // Bullet 個別のロジックがあればここに書く（現状はBulletComponentが移動を担当）
}

void Bullet::Draw(float alpha)
{

    //1) ワールド行列をセット
    Matrix4x4 world = GetTransform().GetMatrix();
    Renderer::SetWorldMatrix(&world);

    //2) 深度テストやカリングはデフォルト（表側のみ描画）でOK
    Renderer::SetDepthEnable(true);
    Renderer::DisableCulling(false);

    //3) 単色を渡すための定数バッファ更新例
    struct ColorCB { DirectX::XMFLOAT4 color; };
    static ID3D11Buffer* colorCB = nullptr;
    if (!colorCB)
    {
        // 一度だけ定数バッファを作成
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(ColorCB);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &colorCB);
    }
    // 好きな色を指定（赤い弾丸など）
    ColorCB cbData{ {1.0f, 0.0f, 0.0f, 1.0f} };
    Renderer::GetDeviceContext()->PSSetConstantBuffers(0, 1, &colorCB);
    Renderer::GetDeviceContext()->UpdateSubresource(colorCB, 0, nullptr, &cbData, 0, 0);

    // 4) プリミティブの描画
    m_primitive.Draw(Renderer::GetDeviceContext());

    // 5) 後片づけ（PS レジスタを戻す）
    ID3D11Buffer* nullCB = nullptr;
    Renderer::GetDeviceContext()->PSSetConstantBuffers(0, 1, &nullCB);

    //std::cout << "Bullet::Drawを実行終了しました。" << std::endl;
}


