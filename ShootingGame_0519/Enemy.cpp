#include <iostream>
#include "Enemy.h"
#include "Bullet.h"

void Enemy::Initialize()
{
    //基底クラスの初期化（Component群）
    GameObject::Initialize();

    //モデルコンポーネントの生成
    auto modelComp = std::make_shared<ModelComponent>();

    //モデルの読み込み（失敗時に備えログなども可）
    modelComp->LoadModel("Asset/Model/Robot/uploads_files_3862208_Cube.fbx");

    SetRotation(DirectX::SimpleMath::Vector3(0.0, 0.0, 0.0));

    //コライダーコンポーネントの生成
    m_Collider  = std::make_shared<OBBColliderComponent>();
    m_Collider -> SetSize({ 2.0f, 2.0f, 2.0f }); // モデルに合わせて調整
    
    //---------------GameObjectに追加---------------
    AddComponent(modelComp);
    AddComponent(m_Collider);
    //----------------------------------------------
}

void Enemy::Update(float dt)
{
    //コンポーネント等のUpdate
    GameObject::Update(dt);
}

//衝突時の処理
void Enemy::OnCollision(GameObject* other)
{
    if (!other) return;
    // 弾と衝突したら自分と弾を消す
    if (dynamic_cast<Bullet*>(other))
    {
        IScene* scene = GetScene();
        if (scene)
        {
            std::cout << "Bulletに衝突したためEnemyを削除します " << std::endl;
            scene->RemoveObject(this);    // raw pointer 版（IScene に追加済み）
            scene->RemoveObject(other);   // 衝突相手も消す
        }
    }
}

//～dynamic_castとは～
//基底クラスと派生クラスのポインタ/参照の変換
//実行時に型のチェックをし、失敗したときは nullptr(ポインタの場合)になる
//ポリモーフィズムが有効な型(virtual 必須)でしか使えない
//上記の場合だとGameObject*をBullet*に変換しています