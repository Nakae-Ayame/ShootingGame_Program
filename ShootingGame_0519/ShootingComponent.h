#pragma once
#include "Component.h"
#include "IScene.h"
#include "ICameraViewProvider.h"
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Bullet; // 前方宣言

class ShootingComponent : public Component
{
public:
    ShootingComponent() = default;
    ~ShootingComponent() override = default;

    //初期化関数
    void Initialize() override {}
    //更新関数
    void Update(float dt) override;

    //シーンとカメラを外部からセット
    void SetScene(IScene* scene) { m_scene = scene; }
    void SetCamera(ICameraViewProvider* camera) { m_camera = camera; }

    // 設定
    void SetCooldown(float cd) { m_cooldown = cd; }
    void SetBulletSpeed(float sp) { m_bulletSpeed = sp; }
    void SetSpawnOffset(float off) { m_spawnOffset = off; }
    const std::vector<std::weak_ptr<GameObject>>& GetSelectedTargets() const { return m_selectedTargets; }

private:
    //弾生成関数(GameObjectを返します)
    std::shared_ptr<GameObject> CreateBullet(const Vector3& pos, const Vector3& dir);

    //シーンとカメラを残しておく変数
    IScene* m_scene = nullptr;
    ICameraViewProvider* m_camera = nullptr;

    float m_cooldown = 0.1f;   // 発射間隔（秒）
    float m_timer = 0.0f;
    float m_bulletSpeed = 220.0f;
    float m_spawnOffset = 1.5f; // プレイヤー前方に出現させるオフセット

    //--- 選択モード関連 ---
    bool  m_canSelect = true;        
    float m_selectionCooldown = 3.0f; //追尾弾発射後のクールダウン
    float m_selectionTimer = 0.0f;    //

    bool m_xWasDown = false;

    int m_maxTargets = 3;              //選択できるターゲット
    float m_selectionRadiusPx = 48.0f; //レティクルからの許容距離（px）

    // 選択判定用の世界単位の半径（敵の中心からの距離がこの半径以下ならヒット）
    float m_selectionRadiusWorld = 1.0f; // デフォルト: 1.0 world units

    // レイの最大長さ（これ以上先は判定しない）
    float m_rayMaxDist = 100.0f; // デフォルト: 100 units

    std::vector<std::weak_ptr<GameObject>> m_selectedTargets; //選択中のターゲット

    float m_homingStrength = 8.0f;

    float m_ScreenOffsetScale = 8.0f; 
    float m_MaxScreenOffset = 12.0f; //Playerが曲がる時にズレる量
};


