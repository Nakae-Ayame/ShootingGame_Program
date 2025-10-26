//#pragma once
//#include "GameObject.h"
//#include "ModelComponent.h"
//#include "OBBColliderComponent.h"
//#include <memory>
//
//class StaticTurretEnemy : public GameObject
//{
//public:
//    StaticTurretEnemy() = default;
//    ~StaticTurretEnemy() override = default;
//
//    void Initialize() override;
//    void Update(float dt) override;
//    void OnCollision(GameObject* other) override;
//
//    // •Ö—˜ƒZƒbƒg
//    void SetCooldown(float sec);
//    void SetBulletSpeed(float speed);
//    void SetSpawnOffset(float offset);
//    void SetMinDistanceToStopShooting(float d);
//
//private:
//    std::shared_ptr<ModelComponent> m_model;
//    std::shared_ptr<EnemyTurretComponent> m_turret;
//    std::shared_ptr<OBBColliderComponent> m_collider;
//};