#include "Bullet.h"
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

    float radius = m_radius; // もし m_radius を外部から変えたいならここを書き換え
    m_bullet = AddComponent<SphereComponent>(radius, 8, 8);
    if (m_bullet) m_bullet->Initialize();
    //------------------

    if (m_collider)
    {
        // OBBColliderComponent::SetSize() は全体サイズ（幅・高さ・奥行き）を期待する想定
        m_collider->SetSize(Vector3(m_radius * 2.0f, m_radius * 2.0f, m_radius * 2.0f));

        // CollisionManager に登録する（フレーム毎に登録/クリアする実装なら不要）
        CollisionManager::RegisterCollider(m_collider.get());
    }
}

void Bullet::Update(float dt)
{
    // 基底のコンポーネント Update を呼ぶ（GameObject::Update がコンポーネント群を回す実装をしている想定）
    GameObject::Update(dt); // もし GameObject::Update(float) に変えたならそちらを呼んでください

    // Bullet 個別のロジックがあればここに書く（現状はBulletComponentが移動を担当）
}

void Bullet::Draw(float dt)
{
    // 基底のコンポーネント Update を呼ぶ（GameObject::Update がコンポーネント群を回す実装をしている想定）
    GameObject::Draw(dt); // もし GameObject::Update(float) に変えたならそちらを呼んでください
}


