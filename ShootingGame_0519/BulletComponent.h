#pragma once
#include "Component.h"
#include <SimpleMath.h>
#include <memory> 

using namespace DirectX::SimpleMath;

class GameObject; // 前方宣言

class BulletComponent : public Component
{
public:
    //弾の種類のENUM
    enum BulletType
    {
        UNKNOW,
        PLAYER,
        ENEMY
    };

    BulletComponent() = default;
    ~BulletComponent() override = default;

    //初期化関数
    void Initialize() override;

    //更新関数
    void Update(float dt) override;

    //---------------------セッターまとめ-----------------------
    //方向ベクトル
    void SetVelocity(const Vector3& v) { m_velocity = v; }
    //現在の方向ベクトル取得
    Vector3 GetVelocity() const { return m_velocity; }

    //スピード
    void SetSpeed(float s) { m_speed = s; }
    float GetSpeed() const { return m_speed; }

    //弾の生存時間
    void SetLifetime(float sec) { m_lifetime = sec; }
    float GetLifetime() const { return m_lifetime; }

    //弾の種類
    void SetBulletType(BulletType t) { m_ownerType = t; }
    BulletType GetBulletType() const { return m_ownerType; }

    //色（見た目）
    void SetColor(const Vector4& c) { m_color = c; }
    Vector4 GetColor() const { return m_color; }

    //---------------------その他-----------------------
    // 将来的にCollisionコールバック等が必要ならここに追加

private:
    //--------------物理関連------------------
    Vector3 m_velocity = Vector3::Zero; //方向ベクトル(単位ベクトルが望ましい)
    float m_speed = 40.0f;              //スピード

    //--------------寿命関連------------------
    float m_age = 0.0f;                 //経過時間
    float m_lifetime = 3.0f;            //生存時間

    //--------------メタ情報------------------
    BulletType m_ownerType = BulletType::UNKNOW;  //弾の種類

    //--------------見た目関連------------------
    Vector4 m_color = Vector4(1, 1, 1, 1);  //色(RGBA)
};
