#include <iostream>
#include "GameScene.h"
#include "Input.h"
#include "DebugGlobals.h" 
#include "renderer.h"
#include "Application.h"
#include "Collision.h"
#include "CollisionManager.h"
#include "SkyDome.h"

void GameScene::Init()
{
    std::cout << "[GS] Init start" << std::endl;
    
    //-----------------------スカイドーム作成-------------------------------
    m_SkyDome = std::make_shared<SkyDome>("Asset/SkyDome/SkyDome_02.png");
    m_SkyDome->Initialize();

    //--------------------------プレイヤー作成---------------------------------
    m_player = std::make_shared<Player>();
    m_player->SetPosition({ 0.0f, 0.0f, 0.0f });
    m_player->SetRotation({ 0.0f,0.0f,0.0f });
    m_player->SetScale({ 0.3f, 0.3f, 0.3f });
    m_player->Initialize();
    auto moveComp = m_player->GetComponent<MoveComponent>();

    //--------------------------エネミー作成---------------------------------
    m_enemy01 = std::make_shared<Enemy>();
    m_enemy01->SetPosition({ 10.0f, 0.0f, 0.0f });
    m_enemy01->SetRotation({ 0.0f,0.0f,0.0f });
    m_enemy01->SetScale({ 1.0f, 1.0f, 1.0f });
    m_enemy01->Initialize();

    //-------------------------レティクル作成-------------------------------------
    m_reticle = std::make_shared<Reticle>(L"Asset/UI/26692699.png", m_reticleW);
    RECT rc{};
    GetClientRect(Application::GetWindow(), &rc);
    float x = (rc.right - rc.left) / 2;
    float y = (rc.bottom - rc.top) / 2;
    m_reticle->Initialize();

    //------------------------------追尾カメラ作成---------------------------------
    m_FollowCamera = std::make_shared<CameraObject>();
    m_FollowCamera->Initialize();
    
    auto shootComp = m_player->GetComponent<ShootingComponent>();
    //ShootingComponent に this（現在のシーン）を渡す
    if (shootComp)
    {
        shootComp->SetScene(this);
    }
   
    auto cameraComp = m_FollowCamera->GetComponent<FollowCameraComponent>();

    if (moveComp && cameraComp)
    {
        moveComp->SetCameraView(cameraComp.get()); 
    }

    if (shootComp && cameraComp)
    {
        shootComp->SetScene(this);
        shootComp->SetCamera(cameraComp.get());
    }

    if (cameraComp)
    {
        m_SkyDome->SetCamera(cameraComp.get()); //ICameraViewProvider* を受け取る場合
    }
    m_GameObjects.insert(m_GameObjects.begin(), m_SkyDome);

    m_FollowCamera->GetCameraComponent()->SetTarget(m_player.get());

    m_GameObjects.push_back(m_player);
    m_GameObjects.push_back(m_enemy01);
    m_GameObjects.push_back(m_reticle);
    m_GameObjects.push_back(m_FollowCamera);

    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        Vector2 screenPos(static_cast<float>(m_lastDragPos.x), static_cast<float>(m_lastDragPos.y));
        m_FollowCamera->GetCameraComponent()->SetReticleScreenPos(screenPos);
    }
}

void GameScene::Update(float deltatime)
{
    //新規オブジェクトをGameSceneのオブジェクト配列に追加する
    SetSceneObject();

    //----------------- レティクルのドラッグ処理 -----------------
    if (Input::IsMouseLeftPressed())
    {
        m_isDragging = true;
    }

    if (m_isDragging && Input::IsMouseLeftDown()) 
    {
        m_lastDragPos = Input::GetMousePosition();
        SetReticleByCenter(m_lastDragPos);
    }

    if (!Input::IsMouseLeftDown() && m_isDragging)
    {
        m_isDragging = false;
    }

    //カメラにレティクル座標を渡す（最新のものを渡す）
    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        m_FollowCamera->GetCameraComponent()->SetReticleScreenPos(Vector2((float)m_lastDragPos.x, (float)m_lastDragPos.y));
    }

    //全オブジェクト Update を一回だけ実行（重要）
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Update(deltatime);
    }

    for (auto& obj : m_GameObjects)
    {
        if (!obj) continue;
        auto collider = obj->GetComponent<ColliderComponent>();
        if (collider)
        {
            CollisionManager::RegisterCollider(collider.get());
        }
    }

    
    Vector3 ppos = m_player->GetPosition();
}

void GameScene::Draw(float deltatime)
{
    // 1) 通常のオブジェクト描画（Reticle をここで skip）
    for (auto& obj : m_GameObjects)
    {

        if (!obj) continue;
        if (std::dynamic_pointer_cast<Reticle>(obj)) continue; // HUD はあとで描く
        obj->Draw(deltatime);
    }

    // 3) HUD（レティクル）を最後に描く（深度やブレンド切り替えは内部で処理）
    if (m_reticle)
    {
        m_reticle->Draw(deltatime);
    }
    
    //デバッグ線
    //CollisionManager::DebugDrawAllColliders(gDebug);

    // （もし m_reticleTex 経由の旧描画があるなら、それも最後に）
    if (m_reticleTex && m_reticleTex->GetSRV())
    {
        Vector2 size(m_reticleW, m_reticleH);
        Renderer::DrawReticle(m_reticleTex->GetSRV(), m_lastDragPos, size);
    }
}


void GameScene::Uninit()
{
    m_player.reset();
}

void GameScene::AddObject(std::shared_ptr<GameObject> obj)
{
    if (!obj) return;

    // 既にシーン内にいるか pending にいるかチェック
    auto itInScene = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });
    if (itInScene != m_GameObjects.end()) return; // 既に実体がある

    auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });
    if (itPending != m_AddObjects.end()) return; // 追加予定にすでにある

    obj->SetScene(this);
    m_AddObjects.push_back(obj);
}

//void GameScene::RemoveObject(std::shared_ptr<GameObject> obj) 
//{ 
//    if (!obj) return;
//    // 重複 push を防ぐ
//    auto it = std::find_if(m_DeleteObjects.begin(), m_DeleteObjects.end(), [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });
//    if (it != m_DeleteObjects.end()) return;
//    m_DeleteObjects.push_back(obj);
//
//} 

void GameScene::RemoveObject(GameObject* obj)
{
    //ポインタがないなら処理終わり
    if (!obj) return;

    //-------------まずm_AddObjectsにいるか確認-------------
    //-------------------いるなら取り消す-----------------
   auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
       [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj; });
   if (itPending != m_AddObjects.end())
   {
       m_AddObjects.erase(itPending);
       return;
   }
    //-------------次に m_GameObjectsにいるか確認-----------------
    //-------------いるならm_DeleteObjects に登録-----------------
    auto itInScene = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj; });
    if (itInScene != m_GameObjects.end())
    {
        // 二重登録防止
        auto already = std::find_if(m_DeleteObjects.begin(), m_DeleteObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj; });
        if (already == m_DeleteObjects.end())
        {
            m_DeleteObjects.push_back(*itInScene);
        }
        return;
    }
    //------------------------------------------------------------
}

void GameScene::FinishFrameCleanup()
{ 
    // m_DeleteObjects にあるアイテムを m_GameObjects から削除
    for (auto& delSp : m_DeleteObjects)
    {
        if (!delSp) continue;
        // find by pointer equality
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == delSp.get(); });
        if (it != m_GameObjects.end())
        {
            // オプション: Scene 参照をクリアしたい場合
            // (*it)->SetScene(nullptr);

            m_GameObjects.erase(it);
            // shared_ptr をここで破棄 -> 実際の破棄は参照カウント次第
        }
        // もしオブジェクトがまだ m_AddObjects に入っているケースがあるなら（安全策）
        auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == delSp.get(); });
        if (itPending != m_AddObjects.end())
        {
            m_AddObjects.erase(itPending);
        }
    }

    m_DeleteObjects.clear();
} 

void GameScene::SetSceneObject()
{ 
    if (!m_AddObjects.empty())
    { // 一括追加（file-safe: reserve してから insert） 
        m_GameObjects.reserve(m_GameObjects.size() + m_AddObjects.size()); 
        m_GameObjects.insert(m_GameObjects.end(), std::make_move_iterator(m_AddObjects.begin()), std::make_move_iterator(m_AddObjects.end())); 
        m_AddObjects.clear();
    } 
}
