#include "ShootingComponent.h"
#include "Bullet.h"
#include "Input.h"
#include <Windows.h> // VK_SPACE
#include <iostream>

using namespace DirectX::SimpleMath;

void ShootingComponent::Update(float dt)
{
    // タイマー更新
    m_timer += dt;

    if (!GetOwner()) return;

    //クールダウン中ではなくスペースキーで発射
    //(IsKeyDown は押しっぱなし判定)
    if (m_timer >= m_cooldown && Input::IsKeyDown(VK_SPACE))
    {
        //カメラやシーンがセットされていないときは発射出来ない
        if (!m_camera || !m_scene)
        {  
            m_timer = 0.0f; // あるいはリセットしないで次フレーム試す選択もある
            return;
        }

        // カメラの前方ベクトルを取得（ワールド空間）
        Vector3 forward = m_camera->GetForward();
        if (forward.LengthSquared() < 1e-6f)
        {
            forward = Vector3::UnitZ; // フォールバック
        }

        //正規化
        forward.Normalize();

        //弾の発生位置：所有者の位置 + forward * offset
        Vector3 spawnPos = GetOwner()->GetPosition() + forward * m_spawnOffset;

        //弾を作ってシーンに登録
        auto bulletObj = CreateBullet(spawnPos, forward);

        if (bulletObj)
        {
            {
                char buf[128];
                sprintf_s(buf, "Update: before AddObject use_count=%lu\n", (unsigned long)bulletObj.use_count());
                OutputDebugStringA(buf);
            }

            m_scene->AddObject(bulletObj);

            // Add 後ログ
            {
                char buf[128];
                sprintf_s(buf, "Update: after AddObject use_count=%lu\n", (unsigned long)bulletObj.use_count());
                OutputDebugStringA(buf);
            }
        }

        //タイマーをゼロにして
        //クールダウン中は出ないようにする
        m_timer = 0.0f;

        //std::cout << "ログ出力(spawPos):" << spawnPos.x << " , " << spawnPos.y << " , " << spawnPos.z << std::endl;
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
