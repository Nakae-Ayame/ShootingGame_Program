#include "ShootingComponent.h"
#include "Bullet.h"
#include "Input.h"
#include <Windows.h> // VK_SPACE
#include <iostream>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include "Application.h"

using namespace DirectX;

using namespace DirectX::SimpleMath;

void ShootingComponent::Update(float dt)
{
    m_timer += dt;
    if (!GetOwner()) return;

    if (m_timer >= m_cooldown && Input::IsKeyDown(VK_SPACE))
    {
        if (!m_scene || !m_camera)
        {
            m_timer = 0.0f;
            return;
        }

        // --- 1) マウス座標を取得（クライアント座標: 0..width-1, 0..height-1） ---
        POINT mpos = Input::GetMousePosition();
        float sx = (float)mpos.x;
        float sy = (float)mpos.y;

        // 画面サイズ（Renderer/Application）
        float screenW = (float)Application::GetWidth();
        float screenH = (float)Application::GetHeight();

        // --- 2) view/proj の取得（ICameraViewProvider から） ---
        // ICameraViewProvider::GetView() / GetProj() は SimpleMath::Matrix を返す想定
        Matrix view = m_camera->GetView();
        Matrix proj = m_camera->GetProj();

        // XM 用に変換
        XMMATRIX viewXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&view));
        XMMATRIX projXM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&proj));
        XMMATRIX worldXM = XMMatrixIdentity();

        // --- 3) near/far をアンプロジェクト ---
        // 注意: XMVector3Unproject の Z は 0..1 (Direct3D のデフォルト) を使う。
        XMVECTOR nearScreen = XMVectorSet(sx, sy, 0.0f, 1.0f);
        XMVECTOR farScreen = XMVectorSet(sx, sy, 1.0f, 1.0f);

        XMVECTOR nearWorldV = XMVector3Unproject(
            nearScreen,
            0.0f, 0.0f, screenW, screenH,
            0.0f, 1.0f,
            projXM,
            viewXM,
            worldXM);

        XMVECTOR farWorldV = XMVector3Unproject(
            farScreen,
            0.0f, 0.0f, screenW, screenH,
            0.0f, 1.0f,
            projXM,
            viewXM,
            worldXM);

        Vector3 nearWorld(XMVectorGetX(nearWorldV), XMVectorGetY(nearWorldV), XMVectorGetZ(nearWorldV));
        Vector3 farWorld(XMVectorGetX(farWorldV), XMVectorGetY(farWorldV), XMVectorGetZ(farWorldV));

        // --- 4) 方向ベクトルを作って正規化 ---
        Vector3 dir = farWorld - nearWorld;
        const float EPS = 1e-6f;
        if (dir.LengthSquared() < EPS)
        {
            // フォールバック（カメラの forward）
            dir = m_camera->GetForward();
            if (dir.LengthSquared() < EPS) dir = Vector3::UnitZ;
        }
        dir.Normalize();

        // 発射位置（プレイヤーの位置 + 少し前方へオフセット）
        Vector3 spawnPos = GetOwner()->GetPosition() + dir * m_spawnOffset;

        // --- 5) 弾生成 ---
        auto bulletObj = CreateBullet(spawnPos, dir);
        if (bulletObj)
        {
            std::cout << "Spawn bullet at (" << spawnPos.x << "," << spawnPos.y << "," << spawnPos.z
                << ") dir=(" << dir.x << "," << dir.y << "," << dir.z << ")\n";
            m_scene->AddObject(bulletObj);
        }

        m_timer = 0.0f;
    }
}

std::shared_ptr<GameObject> ShootingComponent::CreateBullet(const Vector3& pos, const Vector3& dir)
{
    //GameObjectを継承したBulletを生成
    auto bullet = std::make_shared<Bullet>();

    //初期位置・回転
    bullet->SetPosition(pos);
    OutputDebugStringA("CreateBullet: after SetPosition\n");

    //Initializeしてコンポーネントを構築（Bullet::Initialize 内で Component を追加している想定）
    bullet->Initialize();
    OutputDebugStringA("CreateBullet: after Initialize\n");

    //BulletComponentを取得して初期速度を与える
    auto bc = bullet->GetComponent<BulletComponent>();

    //BulletComponentクラスがあったら
    if (bc)
    {
        Vector3 d = dir;
        if (d.LengthSquared() < 1e-6f)
        {
            d = Vector3::UnitZ;
        }
        d.Normalize();
        bc->SetVelocity(d);
        bc->SetSpeed(m_bulletSpeed);

    }
    return bullet;
}
