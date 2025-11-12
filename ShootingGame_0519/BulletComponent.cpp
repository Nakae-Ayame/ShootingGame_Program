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
    if (!GetOwner())
    {
        return;
    }

    //生存時間更新
    m_age += dt;

    //生存時間が寿命を超えていたら
    if (m_age >= m_lifetime)
    {
        //このクラスを所持しているオブジェクトにアクセス
        GameObject* owner = GetOwner();

        //ちゃんと存在していたら
        if (owner)
        {
            //オブジェクトが所属しているSceneにアクセス
            IScene* s = owner->GetScene();

            //ちゃんと存在していたら
            if (s)
            {
                //オブジェクト削除候補の配列に入れる
                s->RemoveObject(owner);
                //std::cout << "弾の稼働時間が終了したため削除します " << std::endl;
            }
        }
        return; // 以降の更新はしない
    }

    //追尾弾の処理:ターゲットの向きに修正
    auto targetSp = m_target.lock();
    //
    if (targetSp)
    {
        //弾自身の位置を取ってくる
        Vector3 myPos = GetOwner()->GetPosition();

        //ターゲットの位置を取ってくる
        Vector3 toTarget = targetSp->GetPosition() - myPos;

        //
        if (toTarget.LengthSquared() > 1e-6f)
        {
            toTarget.Normalize();
            // 補間：現在の m_velocity の方向をホーミング強度で寄せる
            Vector3 currentDir = m_velocity;
            if (currentDir.LengthSquared() < 1e-6f) currentDir = Vector3::Forward;
            else currentDir.Normalize();

            // Lerp-like blending
            float alpha = std::clamp(m_homingStrength * dt, 0.0f, 1.0f);
            Vector3 newDir = currentDir + (toTarget - currentDir) * alpha;
            if (newDir.LengthSquared() > 1e-6f) newDir.Normalize();
            m_velocity = newDir;
        }
    }

    // 移動
    Vector3 pos = GetOwner()->GetPosition();
    pos += m_velocity * m_speed * dt;
    GetOwner()->SetPosition(pos);

    // デバッグログ（任意、頻度落とすべし）
    static float accum = 0.0f;
    accum += dt;
    if (accum > 1.0f)
    { // 1秒ごと
        accum = 0.0f;
        //std::cout << "[Bullet] pos=(" << pos.x << "," << pos.y << "," << pos.z << ") speed=" << m_speed << "\n";
    }
}
