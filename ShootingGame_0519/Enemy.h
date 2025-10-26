#pragma once
#include "GameObject.h"
#include "ModelComponent.h"
//#include "EnemyTurretComponent.h"
#include "AABBColliderComponent.h"
#include "OBBColliderComponent.h"

class Enemy : public GameObject
{
public:
    Enemy() = default;
    ~Enemy() override = default;

    //初期化
    void Initialize() override;   

    //更新
    void Update(float dt) override;   
    
    //HP初期値設定用
    void SetInitialHP(int hp) { m_hp = hp; }

    //ダメージ
    virtual void Damage(int amount);  

    //回復
    virtual void Heal(int amount);     

    //デス判定
    bool IsAlive() const { return m_hp > 0; }   

    //死んだときの処理
    virtual void OnDeath();     

    //衝突処理
    void OnCollision(GameObject* other) override; 

protected:
   

    //削除用関数
    void RemoveSelfFromScene();

    //デス時に呼ぶ共通処理
    virtual void HandleDeathCommon();

private:
    int m_hp = 1;
};


