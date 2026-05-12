#pragma once
#include "GameObject.h"
#include "ModelComponent.h"
#include "AABBColliderComponent.h"
#include "OBBColliderComponent.h"


class Enemy : public GameObject
{
public:
    Enemy() = default;
    ~Enemy() override = default;

    void Initialize() override;   
    void Update(float dt) override;   
    
	//-------------------Set関数-------------------
    void SetInitialHP(int hp) { m_hp = hp; }
    void SetBoundingRadius(float r) { m_boundingRadius = r; }
    void SetOnReturnedToPool(const std::function<void(Enemy*)>& onReturned);
    
	//-------------------Get関数-------------------
    float GetBoundingRadius() const { return m_boundingRadius; }

    void ActivateEnemy(const DirectX::SimpleMath::Vector3& pos);
    void DeactivateEnemy();
    
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

    //-------------------Set関数-------------------
    void SetOnDeathCallback(const std::function<void(Enemy*)>& callback);
protected:
   

    //削除用関数
    void RemoveSelfFromScene();

    //デス時に呼ぶ共通処理
    virtual void HandleDeathCommon();

private:
    int m_hp = 1;
    float m_boundingRadius = 1.0f; // デフォルト
    std::function<void(Enemy*)> m_onReturnedToPool;
    bool m_isDead = false;

    //--------------死亡通知関連------------------
    std::function<void(Enemy*)> m_onDeathCallback;
};


