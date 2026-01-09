#include "BulletComponent.h"
#include "GameObject.h"
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

    //移動（Homing は外部コンポーネントが velocity を調整する前提）
    Vector3 pos = GetOwner()->GetPosition();
    pos += m_velocity * m_speed * dt;
    GetOwner()->SetPosition(pos);
}
