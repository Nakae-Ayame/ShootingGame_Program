#include "Player.h"
#include "Building.h"
#include "ModelComponent.h"
#include "MoveComponent.h"
#include "ShootingComponent.h"
#include "OBBColliderComponent.h"
#include "HitPointCompornent.h"
#include "BulletComponent.h"
#include "Collision.h" 
#include "Enemy.h"
#include "Sound.h"
#include "TextureManager.h"
#include "PushOutComponent.h"
#include <iostream>

void Player::Initialize()
{
    //モデルコンポーネントの生成
    auto modelComp = std::make_shared<ModelComponent>();
    //モデルの読み込み
    modelComp->LoadModel("Asset/Model/Player/Fighterjet.obj");
    modelComp->SetColor(Color(1, 0, 0, 1));

    //移動コンポーネントの生成
    auto moveComp = std::make_shared<MoveComponent>();
    //弾発射コンポーネントの生成
    auto shootComp = std::make_shared<ShootingComponent>();
    //HPコンポーネントの生成
    auto HPComp = std::make_shared<HitPointComponent>(20);
    HPComp->SetInvincibilityOnHit(1.5f);

    auto push = std::make_shared<PushOutComponent>();
    push->SetMass(2.0f);

    //コライダーコンポーネントの生成
    m_Collider = std::make_shared<OBBColliderComponent>();
    m_Collider -> SetSize({ 6.0f, 1.5f, 8.0f }); // モデルに合わせて調整
    m_Collider ->isStatic = false;

  //---------------GameObjectに追加---------------
    AddComponent(modelComp);
    
    AddComponent(shootComp);
    AddComponent(moveComp);
    
    AddComponent(HPComp);
    AddComponent(m_Collider);
    AddComponent(push);
  //----------------------------------------------
    
    SetPosition({ 0.0f, 0.0f, -350.0f });
    SetRotation({ 0.0,60.0,0.0 });
    SetScale({ 1.0f, 1.0f, 1.0f });
    GameObject::Initialize();
}

void Player::Update(float dt)
{
    //コンポーネント等のUpdate
    GameObject::Update(dt);
}

void Player::OnCollision(GameObject* other)
{
    if (!other) { return; }

    //--------弾の判定処理-----------
    if (auto bc = other->GetComponent<BulletComponent>())
    {
        if (bc->GetBulletType() == BulletComponent::BulletType::ENEMY)
        {
            //敵弾向こうに任せる
            auto hp = GetComponent<HitPointComponent>();
            if (hp)
            {
                DamageInfo di;
                di.amount = 2;
                di.instigator = other;
                di.tag = "enemy_bullet";
                bool applied = hp->ApplyDamage(di);
                if (applied)
                {
                    // 弾は消す
                    if (auto s = GetScene())
                    {
                        s->RemoveObject(other);
                    }
                }
            }
            else
            {
                // 互換: 旧処理（即死）
                if (auto s = GetScene())
                {
                    s->RemoveObject(this);
                    s->RemoveObject(other);
                }
            }
            return;
        }
    }

    //--------建物衝突処理--------
    if (auto b = dynamic_cast<Building*>(other))
    {
        //HitPointComponentにダメージを与える
        auto hp = GetComponent<HitPointComponent>();

        if (hp)
        {
            DamageInfo di;
            di.amount = 4;
            di.instigator = other;
            di.tag = "collision_building";
            hp->ApplyDamage(di);
        }
        return;
    }

    //--------敵衝突処理--------
    if (auto enemy = dynamic_cast<Enemy*>(other))
    {
        std::cout << "[Player] Collide Enemy : " << enemy << "\n";

        auto hp = GetComponent<HitPointComponent>();
        if (hp)
        {
            DamageInfo di;
            di.amount = 4;
            di.instigator = other;
            di.tag = "collision_enemy";
            bool applied = hp->ApplyDamage(di);

            std::cout << "  applied = " << applied << "\n";

            if (applied)
            {
                DirectX::SimpleMath::Vector3 dir = GetPosition() - other->GetPosition();
                dir.y = 0.0f;

                if (dir.LengthSquared() < 1e-6f)
                {
                    int r = std::rand() % 2;
                    if (r == 0)
                    {
                        dir = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f);
                    }
                    else
                    {
                        dir = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
                    }
                }

                dir.Normalize();

                DirectX::SimpleMath::Vector3 lateral = DirectX::SimpleMath::Vector3(-dir.z, 0.0f, dir.x);
                lateral.Normalize();

                float sign = (std::rand() % 2 == 0) ? 1.0f : -1.0f;
                const float impulseStrength = 3.0f;
                DirectX::SimpleMath::Vector3 impulse = lateral * sign * impulseStrength;

                std::cout << "  impulse = "
                    << impulse.x << ", "
                    << impulse.y << ", "
                    << impulse.z << "\n";

                auto mv = GetComponent<MoveComponent>();
                if (mv)
                {
                    mv->AddImpulse(impulse);
                }
            }
        }

        return;
    }
}