#include "BulletComponent.h"
#include "GameObject.h"
#include "IScene.h"
#include <iostream> // ログ用（任意）

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
        m_velocity = Vector3::Forward; // デフォルト前方
    }

    //スピードがおかしな数値な場合、当たり障りのない数値を入れる
    if (m_speed <  0)
    {
        m_speed = 20;
    }

}

void BulletComponent::Update(float dt)
{
    //装着しているオブジェクトがないなら更新は終了
    if (!GetOwner()) return;

    //生存時間更新
    m_age += dt;

    //生存時間が寿命を超えていたら
    if (m_age >= m_lifetime)
    {
       //消す処理
    }

    // 速度ベクトルで移動（m_velocity は方向、m_speed は速さ）
    if (m_velocity.LengthSquared() > 0.0f)
    {
        Vector3 pos = GetOwner()->GetPosition();
        pos += m_velocity * m_speed * dt; // dt 秒だけ移動
        GetOwner()->SetPosition(pos);
    }
}
