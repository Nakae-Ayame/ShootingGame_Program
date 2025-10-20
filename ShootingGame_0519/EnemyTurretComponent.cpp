#include <algorithm>
#include "EnemyTurretComponent.h"
#include "Bullet.h"
#include "BulletComponent.h"
#include "GameObject.h"
#include "IScene.h"

void EnemyTurretComponent::Update(float dt)
{
    if (!GetOwner())
    {
        return;
    }

    m_timer += dt;

    if (!m_target)
    {
        return;
    }

    Vector3 myPos = GetOwner()->GetPosition();      //自分の位置を持ってくる
    Vector3 targetPos = m_target->GetPosition();    //ターゲットの位置を持ってくる
    Vector3 toTarget = targetPos - myPos;           //　
    float distSq = toTarget.LengthSquared();
   
    //近すぎたら撃たない(固定砲台の仕様)
    if (distSq < m_minDist * m_minDist)
    {
        
    }

    //回転だけ(見た目) - yaw(y回転)を合わせる。
    if (toTarget.LengthSquared() > 1e-6f)
    {
        Vector3 dir = toTarget; dir.Normalize();
        float targetYaw = std::atan2(dir.x, dir.z);
        Vector3 rot = GetOwner()->GetRotation();
        rot.y = targetYaw; // yaw
        GetOwner()->SetRotation(rot);
    }

    //発射
    if (m_timer >= m_cooldown && distSq >= m_minDist * m_minDist)
    {
       
        //auto bulletObj = std::make_shared<Bullet>();
        //Vector3 dir = toTarget;
        //if (dir.LengthSquared() < 1e-6f)
        //{
        //    dir = Vector3(0, 0, 1);
        //}
        //dir.Normalize();
        //Vector3 spawnPos = myPos + dir * m_spawnOffset;
        //bulletObj->SetPosition(spawnPos);
        //bulletObj->Initialize();
       
        //if (auto bc = bulletObj->GetComponent<BulletComponent>())
        //{
        //    bc->SetVelocity(dir);
        //    bc->SetSpeed(m_bulletSpeed);
        //    bc->SetBulletType(BulletComponent::BulletType::ENEMY);
        //    bc->SetLifetime(5.0f);
        //}
        //
        ////敵のいるSceneを取ってくる
        //if (auto scene = GetOwner()->GetScene())
        //{
        //    //あればそのSceneにセットする
        //    scene->AddObject(bulletObj);
        //}

        ////弾を撃ったのでリセットする
        //m_timer = 0.0f;
    }
}
