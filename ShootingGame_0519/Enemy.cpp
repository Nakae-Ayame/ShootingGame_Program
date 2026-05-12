#include <iostream>
#include "Enemy.h"
#include "Bullet.h"
#include "HitPointCompornent.h"
#include "PatrolComponent.h"
#include "EffectManager.h"

void Enemy::Initialize()
{
    GameObject::Initialize();
}

void Enemy::Update(float dt)
{
    auto hp = GetComponent<HitPointComponent>();
    GameObject::Update(dt);
}

void Enemy::ActivateEnemy(const DirectX::SimpleMath::Vector3& pos)
{
    m_isDead = false;

    SetPosition(pos);
    SetActive(true);

    if (auto hp = GetComponent<HitPointComponent>())
    {
        hp->SetMaxHP(1.0f);
    }

    if (auto col = GetComponent<ColliderComponent>())
    {
        col->SetEnabled(true);
    }
}

void Enemy::DeactivateEnemy()
{
    SetActive(false);

    if (auto col = GetComponent<ColliderComponent>())
    {
        col->SetEnabled(false);
    }
}

//衝突時の処理
void Enemy::OnCollision(GameObject* other)
{
    if (!other){ return; }

    //相手が弾かどうか
    if (auto bulletComp = other->GetComponent<BulletComponent>()) 
    {
        if (bulletComp->GetBulletType() == BulletComponent::BulletType::PLAYER)
        { 

            Sound::PlaySeWav(L"Asset/Sound/SE/Bullet_Hit01.wav", 0.3f);

            auto hp = GetComponent<HitPointComponent>();

            if (!hp) { return; };
            
            DamageInfo di;
            di.amount = 1;
            di.instigator = other;
            di.tag = "player_bullet";
            bool applied = hp->ApplyDamage(di);

            if (hp->GetHP() <= 0)
            {
                OnDeath();

                if (auto scene = GetScene())
                {
                    scene->RemoveObject(other);
                }
            }
        }
    }
}

void Enemy::Damage(int amount)
{
    if (amount <= 0) { return; }
    m_hp -= amount;

    if (m_hp <= 0)
    {
        OnDeath();
    }
}

void Enemy::Heal(int amount)
{
    if (amount <= 0)
    {
        return;
    }
    m_hp += amount;
}

void Enemy::OnDeath()
{
    if (m_isDead)
    {
        return;
    }

    m_isDead = true;

    EffectManager::SpawnExplosion(GetPosition());

    if (m_onDeathCallback)
    {
        m_onDeathCallback(this);
    }

    DeactivateEnemy();

    if (m_onReturnedToPool)
    {
        m_onReturnedToPool(this);
    }
}

void Enemy::HandleDeathCommon()
{
    //エフェクトを追加する
    /*EffectManager::SpawnExplosion(GetPosition(), 8.0f);
    RequestDestroy();*/
    RemoveSelfFromScene();
}

void Enemy::RemoveSelfFromScene()
{
    IScene* s = GetScene();
    if (s)
    {
        s->RemoveObject(this);
    }
    else
    {
        // シーンがない場合はログだけ（安全のため）
        // std::cout << "Warning: Enemy removed but no scene attached.\n";
    }
}

void Enemy::SetOnReturnedToPool(const std::function<void(Enemy*)>& onReturned)
{
    m_onReturnedToPool = onReturned;
}

//～dynamic_castとは～
//基底クラスと派生クラスのポインタ/参照の変換
//実行時に型のチェックをし、失敗したときは nullptr(ポインタの場合)になる
//ポリモーフィズムが有効な型(virtual 必須)でしか使えない
//上記の場合だとGameObject*をBullet*に変換しています
//(今は使ってないです)

void Enemy::SetOnDeathCallback(const std::function<void(Enemy*)>& callback)
{
    m_onDeathCallback = callback;
}