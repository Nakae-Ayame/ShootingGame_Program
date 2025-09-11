#include "ShootingComponent.h"
#include "Bullet.h"
#include "Input.h"
#include <Windows.h> // VK_SPACE

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
            m_scene->AddObject(bulletObj);
        }

        //タイマーをゼロにして
        //クールダウン中は出ないようにする
        m_timer = 0.0f;
    }
}

std::shared_ptr<GameObject> ShootingComponent::CreateBullet(const Vector3& pos, const Vector3& dir)
{
    //GameObjectを継承したBulletを生成
    auto bullet = std::make_shared<Bullet>();

    //初期位置・回転
    bullet->SetPosition(pos);

    //Initializeしてコンポーネントを構築（Bullet::Initialize 内で Component を追加している想定）
    bullet->Initialize();

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

    // ここでビジュアルコンポーネント（ModelComponent など）やコライダーの追加は
    // Bullet::Initialize 内で完結している想定です。

    return bullet;
}
