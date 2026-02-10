#pragma once
#include "Component.h"
#include "IScene.h"
#include "ICameraViewProvider.h"
#include "DebugRenderer.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class GameObject;
class Bullet;
class BulletComponent;

class ShootingComponent : public Component
{
public:
    ShootingComponent() {};

    void Update(float dt) override;

    //------------Set関数--------------
    void SetScene(IScene* scene) { m_scene = scene; }
    void SetCamera(ICameraViewProvider* camera) { m_camera = camera; }

    void SetBulletSpeed(float sp) { m_bulletSpeed = sp; }
    void SetCooldown(float cd) { m_cooldown = cd; }
    void SetSpawnOffset(float off) { m_spawnOffset = off; }
    void SetAutoFire(bool v) { m_autoFire = v; }
    void SetNormalBulletColor(const Vector4& c) { m_normalBulletColor = c; }
    void SetHomingBulletColor(const Vector4& c) { m_homingBulletColor = c; }

	//------------Get関数--------------


    //--------その他メンバ関数-------
    void Fire();

protected:

    std::shared_ptr<GameObject> CreateBullet(const Vector3& pos, const Vector3& dir,
                                             const Vector4& color = Vector4(1,1,1,1));

    void AddBulletToScene(const std::shared_ptr<GameObject>& bullet);

    std::shared_ptr<GameObject> FindBestHomingTarget();

    void FireHomingBullet(GameObject* owner, const std::shared_ptr<GameObject>& targetSp);

private:
    IScene* m_scene = nullptr;                  //生成した弾を追加するシーン
    ICameraViewProvider* m_camera = nullptr;    //発射する向きを取得するカメラ関数

    float m_cooldown = 0.1f;        //クールタイム
    float m_timer = 0.0f;           //経過時間
    float m_bulletSpeed = 300.0f;  //弾の速さ
    float m_spawnOffset = 2.8f;     //発射位置のオフセット
    
    bool m_autoFire = false;   //自動発射モード

    //ホーミングの強さ
    float m_homingStrength = 8.0f;

    //通常弾とホーミング弾で色を分ける
    Vector4 m_normalBulletColor = Vector4(1, 1, 1, 1);          // 白っぽい
    Vector4 m_homingBulletColor = Vector4(1, 0.3f, 0.1f, 1.0f); // 赤っぽい

    bool  m_prevHomingKeyDown = false;
    float m_homingCurveAmount = 0.5f;

    Vector3 m_muzzleLocalOffset = Vector3(0.0f, 0.0f, 2.0f);
};


