//#include "TurretEnemy.h"
//#include "SceneManager.h"
//#include <iostream>
//
//using namespace DirectX::SimpleMath;
//
//void StaticTurretEnemy::Initialize()
//{
//    GameObject::Initialize();
//
//    // モデル（適宜パス変更）
//    m_model = std::make_shared<ModelComponent>();
//    m_model->LoadModel("Asset/Model/Robot/uploads_files_3862208_Cube.fbx");
//
//    // タレット（発射コンポーネント）
//    m_turret = std::make_shared<EnemyTurretComponent>();
//    // デフォルトパラメータは必要に応じて SetXXXX で上書きされる
//
//    // コライダ（当たり判定）
//    m_collider = std::make_shared<OBBColliderComponent>();
//    m_collider->SetSize(Vector3(2.0f, 2.0f, 2.0f));
//
//    AddComponent(m_model);
//    AddComponent(m_turret);
//    AddComponent(m_collider);
//}
//
//void StaticTurretEnemy::Update(float dt)
//{
//    GameObject::Update(dt);
//    // 固定砲台だがターゲット追尾等の追加ロジックを入れるならここに
//}
//
//void StaticTurretEnemy::OnCollision(GameObject* other)
//{
//    // Enemy::OnCollision に合わせた処理にするか、弾判定をここで行う
//    // 例: 弾に当たったら自身を削除
//    if (!other) return;
//    // (弾判定コードはプロジェクトの BulletComponent に合わせてください)
//    GameObject::OnCollision(other);
//}
//
//void StaticTurretEnemy::SetCooldown(float sec)
//{
//    if (!m_turret) m_turret = AddComponent<EnemyTurretComponent>();
//    m_turret->SetCooldown(sec);
//}
//
//void StaticTurretEnemy::SetBulletSpeed(float speed)
//{
//    if (!m_turret) m_turret = AddComponent<EnemyTurretComponent>();
//    m_turret->SetBulletSpeed(speed);
//}
//
//void StaticTurretEnemy::SetSpawnOffset(float offset)
//{
//    if (!m_turret) m_turret = AddComponent<EnemyTurretComponent>();
//    m_turret->SetSpawnOffset(offset);
//}
//
//void StaticTurretEnemy::SetMinDistanceToStopShooting(float d)
//{
//    if (!m_turret) m_turret = AddComponent<EnemyTurretComponent>();
//    m_turret->SetMinDistanceToStopShooting(d);
//}
