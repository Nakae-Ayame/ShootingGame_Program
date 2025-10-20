#pragma once]
#include <SimpleMath.h>
#include <memory>
#include "Component.h"

using namespace DirectX::SimpleMath;

class EnemyTurretComponent : public Component
{
public:
	EnemyTurretComponent() = default;
	~EnemyTurretComponent() override = default;

    //初期化
	void Initialize() override {};   
    //更新          
	void Update(float deltaTime) override;     
    
    //追跡する相手のセッター
    void SetTarget(GameObject* t) { m_target = t; } 
    //発射間隔のセッター
    void SetCooldown(float cd) { m_cooldown = cd; }  
    //弾の速さのセッター
    void SetBulletSpeed(float s) { m_bulletSpeed = s; }  
    //弾の位置補正のセッター
    void SetSpawnOffset(float off) { m_spawnOffset = off; } 
    //撃たなくなる距離のセッター
    void SetMinDistanceToStopShooting(float d) { m_minDist = d; }   

private:
    GameObject* m_target = nullptr; //追跡するターゲット
    float m_rotSpeed = 0.0f;
    float m_cooldown = 1.0f;        //発射間隔
    float m_timer    = 0.0f;        //タイマー用の変数
    float m_bulletSpeed = 50.0f;    //弾の速さ
    float m_spawnOffset = 1.0f;     //位置補正
    float m_minDist     = 3.0f;     //
};