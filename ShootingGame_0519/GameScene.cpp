#include <iostream>
#include "GameScene.h"
#include "Input.h"
#include "DebugGlobals.h" 
#include "renderer.h"
#include "Application.h"
#include "Collision.h"
#include "CollisionManager.h"

void GameScene::Init()
{
    //--------------------------プレイヤー作成---------------------------------
    m_player = std::make_shared<Player>();
    m_player -> SetPosition({ 0.0f, 0.0f, 0.0f });
    m_player -> SetRotation({ 0.0f,0.0f, 0.0f });
    m_player -> SetScale({ 0.3f, 0.3f, 0.3f });
    m_player -> Initialize();
    auto moveComp = m_player->GetComponent<MoveComponent>();
    //------------------------------------------------------------------------
    
    //--------------------------エネミー作成---------------------------------
    m_enemy = std::make_shared<Enemy>();
    m_enemy->SetPosition({ 10.0f, 0.0f, 0.0f });
    m_enemy->SetRotation({ 0.0f,0.0f, 0.0f });
    m_enemy->SetScale({ 1.0f, 1.0f, 1.0f });
    m_enemy->Initialize();
    //----------------------------------------------------------------------

    // ShootingComponent に this（現在のシーン）を渡す
    auto shootComp = m_player->GetComponent<ShootingComponent>();
    if (shootComp)
    {
        shootComp->SetScene(this);
    }

    //CameraObjectst作成
    m_FollowCamera = std::make_shared<CameraObject>();
    m_FollowCamera ->Initialize();
    auto cameraComp = m_FollowCamera->GetComponent<FollowCameraComponent>();

    if (moveComp && cameraComp)
    {
        moveComp->SetCameraView(cameraComp.get()); // ←重要！
    }

    if (shootComp && cameraComp)
    {
        shootComp->SetScene(this);
        shootComp->SetCamera(cameraComp.get()); // ←これが重要！
    }

    m_FollowCamera->GetCameraComponent()->SetTarget(m_player.get());

    m_GameObjects.push_back(m_player);
    m_GameObjects.push_back(m_enemy);
    m_GameObjects.push_back(m_FollowCamera);

}

void GameScene::Update(uint64_t delta)
{
    Input::Update();

    // 全オブジェクト更新
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Update();
    }

    // 1. 前フレームのコライダー情報をクリア
    CollisionManager::Clear();

    // 2. コライダーを登録
    for (auto& obj : m_GameObjects)
    {
        auto collider = obj->GetComponent<ColliderComponent>();
        if (collider)
        {
            CollisionManager::RegisterCollider(collider.get());
        }
    }

    // 3. 衝突判定を一括実行
    CollisionManager::CheckCollisions();

}


void GameScene::Draw(uint64_t deltatime)
{
    // 1) 通常のオブジェクト描画（既存）
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Draw();
    }

    // 2) CollisionManager に登録されているコライダーから、
    //    DebugRenderer にワイヤー箱を積む（色は IsHitThisFrame() を見て決定する）
    CollisionManager::DebugDrawAllColliders(gDebug);

    // 3) カメラから view/proj を取得（安全チェック）
    Matrix viewMat = Matrix::Identity;
    Matrix projMat = Matrix::Identity;

    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        // あなたの API が GetView()/GetProj() を返す想定
        viewMat = m_FollowCamera->GetCameraComponent()->GetView();
        projMat = m_FollowCamera->GetCameraComponent()->GetProj();
    }
    else
    {
        // カメラが無ければ Renderer に最後に設定した行列を使う手もありますが、
        // とりあえず単位行列で安全に処理させます。
    }

    // 4) デバッグ線を常に見せたい場合は深度テストを OFF にする（好みで）
    //    ここでは「常に見える」挙動をデフォルトにしています。
    Renderer::SetDepthEnable(false);

    // 5) 描画実行
    gDebug.Draw(viewMat, projMat);

    // 6) 深度テストを元に戻す（他に描画が続くなら true に戻す）
    Renderer::SetDepthEnable(true);
}

void GameScene::Uninit()
{
    m_player.reset();
}

void GameScene::AddObject(std::shared_ptr<GameObject> obj)
{
    m_GameObjects.push_back(obj); // m_GameObjects は std::vector<std::shared_ptr<GameObject>>
}