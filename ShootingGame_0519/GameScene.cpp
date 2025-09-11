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
    //-----------------------スカイドーム作成-------------------------------
    m_SkyDome = std::make_shared<SkyDome>("Asset/SkyDome/SkyDome_01.jpg");
    m_SkyDome->Initialize();

    //--------------------------プレイヤー作成---------------------------------
    m_player = std::make_shared<Player>();
    m_player->SetPosition({ 0.0f, 0.0f, 0.0f });
    m_player->SetRotation({ 0.0f,0.0f,0.0f });
    m_player->SetScale({ 0.3f, 0.3f, 0.3f });
    m_player->Initialize();
    auto moveComp = m_player->GetComponent<MoveComponent>();

    //--------------------------エネミー作成---------------------------------
    m_enemy = std::make_shared<Enemy>();
    m_enemy->SetPosition({ 10.0f, 0.0f, 0.0f });
    m_enemy->SetRotation({ 0.0f,0.0f,0.0f });
    m_enemy->SetScale({ 1.0f, 1.0f, 1.0f });
    m_enemy->Initialize();

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
    //-----------------------------------------------------------------------------
    
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
        m_SkyDome->SetCamera(cameraComp.get()); // ICameraViewProvider* を受け取る場合
    }
    m_GameObjects.insert(m_GameObjects.begin(), m_SkyDome);

    m_FollowCamera->GetCameraComponent()->SetTarget(m_player.get());

    //m_GameObjects.push_back(m_SkyDome);
    m_GameObjects.push_back(m_player);
    //m_GameObjects.push_back(m_enemy);
    m_GameObjects.push_back(m_FollowCamera);
    m_GameObjects.push_back(m_reticle);

    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        // SimpleMath::Vector2 へ変換して渡す
        Vector2 screenPos(static_cast<float>(m_lastDragPos.x), static_cast<float>(m_lastDragPos.y));
        m_FollowCamera->GetCameraComponent()->SetReticleScreenPos(screenPos);
    }
}

void GameScene::Update(float deltatime)
{
    SetSceneObject();

    Input::Update();

    //レティクル入力を先に処理（ドラッグ）
    if (Input::IsMouseLeftPressed()) m_isDragging = true;
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

    //衝突処理などは既存ロジック通り（下に続けてください）
    CollisionManager::Clear();
    for (auto& obj : m_GameObjects) 
    {
        auto collider = obj->GetComponent<ColliderComponent>();
        if (collider)
        {
            CollisionManager::RegisterCollider(collider.get());
        }
    }
    CollisionManager::CheckCollisions();

    // ----------------- レティクルのドラッグ処理 -----------------
    // 押した瞬間にドラッグ開始
    if (Input::IsMouseLeftPressed())
    {
        m_isDragging = true;
    }

    // 押している間は追従
    if (m_isDragging && Input::IsMouseLeftDown())
    {
        m_lastDragPos = Input::GetMousePosition(); // GetMousePosition はクライアント座標を返す
        SetReticleByCenter(m_lastDragPos);
    }

    // 離したら固定してドラッグ終了
    if (!Input::IsMouseLeftDown() && m_isDragging)
    {
        m_isDragging = false;
        // m_lastDragPos を保持してその位置で止める
    }
    // ----------------------------------------------------------

    Vector3 ppos = m_player->GetPosition();

}

void GameScene::Draw(float deltatime)
{
    // 1) 通常のオブジェクト描画（既存）
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Draw(deltatime);
    }

    // 2) CollisionManager に登録されているコライダーから、
    //    DebugRenderer にワイヤー箱を積む（色は IsHitThisFrame() を見て決定する）
    CollisionManager::DebugDrawAllColliders(gDebug);

    // 3) カメラから view/proj を取得（安全チェック）
    Matrix viewMat = Matrix::Identity;
    Matrix projMat = Matrix::Identity;

    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        viewMat = m_FollowCamera->GetCameraComponent()->GetView();
        projMat = m_FollowCamera->GetCameraComponent()->GetProj();
    }

    // 4) デバッグ線を常に見せたい場合は深度テストを OFF にする（ここでは既定でOFF）
    Renderer::SetDepthEnable(false);
    gDebug.Draw(viewMat, projMat);
    Renderer::SetDepthEnable(true);

    // 5) レティクルを HUD として描画
    //    m_reticleTex->GetSRV() で SRV を渡す。DrawReticle が深度/ブレンドを切り替えます。
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
    // シーン参照を GameObject に教えておく（後述）
    obj->SetScene(this);
    m_AddObjects.push_back(obj);
}

void GameScene::RemoveObject(std::shared_ptr<GameObject> obj)
{
    m_DeleteObjects.push_back(obj);
}

void GameScene::RemoveObject(GameObject* obj)
{
    //if (!obj) return;
    //// 重複追加を防ぎたい場合チェックしてから push_back してもよい
    //m_DeleteObjects.push_back(obj);
}

void GameScene::FinishFrameCleanup()
{
    for (auto& p : m_DeleteObjects) // p は shared_ptr<GameObject>
    {
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),[&](const std::shared_ptr<GameObject>& sp)
            {return sp == p; });
        if (it != m_GameObjects.end()) m_GameObjects.erase(it);
    }
    m_DeleteObjects.clear();

}

void GameScene::SetSceneObject()
{
    if (!m_AddObjects.empty())
    {
        // 一括追加（file-safe: reserve してから insert）
        m_GameObjects.reserve(m_GameObjects.size() + m_AddObjects.size());
        m_GameObjects.insert(m_GameObjects.end(),
            std::make_move_iterator(m_AddObjects.begin()),
            std::make_move_iterator(m_AddObjects.end()));
        m_AddObjects.clear();
    }

}