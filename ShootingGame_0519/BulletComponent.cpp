#include "BulletComponent.h"
#include "GameObject.h"
#include "EffectManager.h"
#include "IScene.h"
#include "Enemy.h"
#include <iostream>

using namespace DirectX::SimpleMath;

void BulletComponent::Initialize()
{
    //方向ベクトルの正規化などをしておくと安全
    if (m_velocity.LengthSquared() > 1e-6f)
    {
        m_velocity.Normalize();
    }  
    else
    {
        //m_velocity = Vector3::Forward; // デフォルト前方
    }

    //スピードがおかしな数値な場合、当たり障りのない数値を入れる
    if (m_speed <  0)
    {
        m_speed = 20;
    }

}

void BulletComponent::Update(float dt)
{
    //オーナーがなければ更新しない
    if (!GetOwner()) { return; }

    //生存時間更新
    m_age += dt;

    //寿命超過ならシーンから削除
    if (m_age >= m_lifetime)
    {
        GameObject* owner = GetOwner();
        if (owner)
        {
            IScene* s = owner->GetScene();
            if (s)
            {
                s->RemoveObject(owner);
            }
        }
        return; // 以降の更新はしない
    }

    GameObject* owner = GetOwner();

    //移動（Homing は外部コンポーネントが velocity を調整する前提）
    Vector3 prevPos = owner->GetPosition();
    Vector3 pos = prevPos;
    pos += m_velocity * m_speed * dt;
    owner->SetPosition(pos);

	//弾道エフェクトの生成
    Vector3 d = pos - prevPos;
    if (d.LengthSquared() > 1e-6f)
    {
        EffectManager::SpawnBulletTrail(prevPos, pos);
    }
}
