#include <iostream>
#include "DebugScene.h"
#include "Input.h"
#include "DebugGlobals.h" 
#include "renderer.h"
#include "Application.h"
#include "Collision.h"
#include "CollisionManager.h"
#include "SkyDome.h"
#include "HPBar.h"
#include "Building.h"
#include "PatrolComponent.h"
#include "CircularPatrolComponent.h"
#include "FloorComponent.h"
//#include "PlayAreaComponent.h"
#include "HitPointCompornent.h"
#include "PushOutComponent.h"
#include "SphereColliderComponent.h"
#include "EffectManager.h"

#include "IniFile.h"
#include <algorithm>

void DebugScene::LoadPlayerConfigFromIni()
{
    IniFile ini(m_iniPath.c_str());

    m_playerMoveSpeed = ini.ReadFloat("Player", "MoveSpeed", m_playerMoveSpeed);
    m_playerBoostMultiplier = ini.ReadFloat("Player", "BoostMultiplier", m_playerBoostMultiplier);
    m_playerBulletSpeed = ini.ReadFloat("Player", "BulletSpeed", m_playerBulletSpeed);
    m_playerHp = ini.ReadFloat("Player", "HpMax", m_playerHp);
}

void DebugScene::DebugConfigWindow()
{
    ImGui::Begin("Save Window");

    //--------------Player設定関連------------------
    ImGui::Text("Player Config");
    ImGui::Separator();

    ImGui::SliderFloat("Player Speed", &m_playerMoveSpeed, 0.1f, 150.0f);
    ImGui::SliderFloat("Boost Multiplier", &m_playerBoostMultiplier, 1.0f, 5.0f);
    ImGui::SliderFloat("Bullet Speed", &m_playerBulletSpeed, 1.0f, 500.0f);
    ImGui::SliderFloat("Player HP", &m_playerHp, 1.0f, 500.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Save Settings"))
    {
        OutputDebugStringA("Save button pressed!\n");
    }

    ImGui::End();
}


static bool RaySphereIntersect(const DirectX::SimpleMath::Vector3& rayOrigin,
    const DirectX::SimpleMath::Vector3& rayDir,
    const DirectX::SimpleMath::Vector3& sphereCenter,
    float sphereRadius,
    float& outT)
{
    using namespace DirectX::SimpleMath;
    Vector3 m = rayOrigin - sphereCenter;
    float b = m.Dot(rayDir);
    float c = m.LengthSquared() - sphereRadius * sphereRadius;
    if (c > 0.0f && b > 0.0f) { return false; }

    float discr = b * b - c;

    if (discr < 0.0f)
    {
        return false;
    }

    float t = -b - sqrtf(discr);

    if (t < 0.0f)
    {
        t = -b + sqrtf(discr);
    }

    if (t < 0.0f)
    {
        return false;
    }

    outT = t;
    return true;
}

bool DebugScene::Raycast(const DirectX::SimpleMath::Vector3& origin,
    const DirectX::SimpleMath::Vector3& dir,
    float maxDistance,
    RaycastHit& outHit,
    std::function<bool(GameObject*)> predicate,
    GameObject* ignore)
{
    using namespace DirectX::SimpleMath;

    // Normalize dir for distance correctness
    Vector3 ndir = dir;
    if (ndir.LengthSquared() <= 1e-6f)
    {
        return false;
    }
    ndir.Normalize();

    float bestT = std::numeric_limits<float>::infinity();
    std::shared_ptr<GameObject> bestObj;

    for (auto& obj : m_GameObjects)
    {
        if (!obj) { continue; }
        if (obj.get() == ignore) { continue; }

        if (predicate)
        {
            if (!predicate(obj.get()))
            {
                continue;
            }
        }

        float t;
        float radius = 0.0f;

        bool hasSphere = false;

        {
            Enemy* e = dynamic_cast<Enemy*>(obj.get());
            if (e)
            {
                radius = e->GetBoundingRadius();
                hasSphere = true;
            }
        }

        if (hasSphere)
        {
            if (RaySphereIntersect(origin, ndir, obj->GetPosition(), radius, t))
            {
                if (t >= 0.0f && t <= maxDistance && t < bestT)
                {
                    bestT = t;
                    bestObj = obj;
                }
            }
        }
        else
        {

        }
    }

    if (bestObj)
    {
        outHit.hitObject = bestObj;
        outHit.distance = bestT;
        outHit.position = origin + ndir * bestT;
        outHit.normal = (outHit.position - bestObj->GetPosition());
        if (outHit.normal.LengthSquared() > 1e-6f)
        {
            outHit.normal.Normalize();
        }
        return true;
    }

    return false;
}

bool DebugScene::RaycastForAI(const DirectX::SimpleMath::Vector3& origin,
    const DirectX::SimpleMath::Vector3& dir,
    float maxDistance,
    RaycastHit& outHit,
    GameObject* ignore)
{
    using namespace DirectX::SimpleMath;

    Vector3 ndir = dir;
    if (ndir.LengthSquared() <= 1e-6f)
        return false;
    ndir.Normalize();

    float bestT = std::numeric_limits<float>::infinity();
    std::shared_ptr<GameObject> bestObj;

    for (auto& obj : m_GameObjects)
    {
        if (!obj) continue;
        if (obj.get() == ignore) continue;

        //コライダーを持っていないなら無視
        auto obb = obj->GetComponent<OBBColliderComponent>();
        auto aabb = obj->GetComponent<AABBColliderComponent>();
        bool hasCollider = (obb || aabb);

        //Enemy の簡易球 or Building の AABB など…
        float radius = 0.0f;
        bool hasSphere = false;

        if (auto e = dynamic_cast<Enemy*>(obj.get()))
        {
            radius = e->GetBoundingRadius();
            if (radius > 0.0f) hasSphere = true;
        }
        else if (hasCollider)
        {
            //コライダーからざっくり半径を作る（対角長の半分）
            //きっちりした Ray vs OBB/AABB は後で実装でもOK
            Vector3 center = obj->GetPosition();
            Vector3 extents;

            if (obb)
            {
                extents = obb->GetSize() * 0.5f; // size が全長なら半分で半径相当
            }
            else // AABB
            {
                Vector3 size = aabb->GetSize();
                extents = size * 0.5f;
            }

            radius = extents.Length(); // 対角線の長さ ≒ おおざっぱな半径
            hasSphere = (radius > 0.0f);
        }

        if (!hasSphere) continue;

        float t;
        if (RaySphereIntersect(origin, ndir, obj->GetPosition(), radius, t))
        {
            if (t >= 0.0f && t <= maxDistance && t < bestT)
            {
                bestT = t;
                bestObj = obj;
            }
        }
    }

    if (bestObj)
    {
        outHit.hitObject = bestObj;
        outHit.distance = bestT;
        outHit.position = origin + ndir * bestT;

        // 簡易法線（中心からの方向）
        outHit.normal = outHit.position - bestObj->GetPosition();
        if (outHit.normal.LengthSquared() > 1e-6f)
            outHit.normal.Normalize();

        return true;
    }

    return false;
}

void DebugScene::InitializeDebug()
{
    DebugUI::RedistDebugFunction([this]() {DebugConfigWindow(); });

    //DebugRendererの初期化
    m_debugRenderer = std::make_unique<DebugRenderer>();
    m_debugRenderer->Initialize(Renderer::GetDevice(),
        Renderer::GetDeviceContext(),
        L"DebugLineVS.cso", L"DebugLinePS.cso");
}

void DebugScene::InitializePlayArea()
{

}

void DebugScene::InitializePhase()
{
    m_gameState = DebugState::Playing;
    m_countdownRemaining = 4.0f;
}

void DebugScene::InitializeCamera()
{
    m_FollowCamera = std::make_shared<CameraObject>();
    m_cameraComp = m_FollowCamera->AddCameraComponent<FollowCameraComponent>().get();
    AddObject(m_FollowCamera);
    m_FollowCamera->Initialize();

    auto moveComp = m_player->GetComponent<MoveComponent>();
    auto cameraComp = m_FollowCamera->GetComponent<FollowCameraComponent>();
    auto shootComp = m_player->GetComponent<ShootingComponent>();

    if (moveComp && cameraComp)
    {
        moveComp->SetCameraView(cameraComp.get());
    }

    if (shootComp && cameraComp)
    {
        shootComp->SetScene(this);
        shootComp->SetCamera(cameraComp.get());
    }

    if (m_cameraComp)
    {
        //m_cameraComp->SetPlayArea(m_playArea.get());
    }

    Vector3 Ppos = m_player->GetPosition();
    m_FollowCamera->SetPosition({ Ppos.x, Ppos.y, Ppos.z - 20 });
}

void DebugScene::InitializePlayer()
{
    m_player = std::make_shared<Player>();
    m_player->Initialize();

    auto moveComp = m_player->GetComponent<MoveComponent>();
    if (moveComp)
    {
		moveComp->SetSpeed(m_playerMoveSpeed);

        std::cout << "Player Speed: " << m_playerMoveSpeed << std::endl;
    }

    auto shootComp = m_player->GetComponent<ShootingComponent>();
    if (shootComp)
    {
        shootComp->SetScene(this);

		shootComp->SetBulletSpeed(m_playerBulletSpeed);
    }

    auto hpComp = m_player->GetComponent<HitPointComponent>();
    if (hpComp)
    {
		hpComp->SetHP(m_playerHp);
    }

}

void DebugScene::InitializeEnemy()
{

}

void DebugScene::InitializeStageObject()
{
    m_buildingSpawner = std::make_unique<BuildingSpawner>(this);
    BuildingConfig bc;
    bc.modelPath = "Asset/Build/wooden watch tower2.obj";
    bc.count = 0;
    bc.areaWidth = 300.0f;
    bc.areaDepth = 300.0f;
    bc.spacing = 30.0f;          //建物間に20単位の余裕を入れる
    bc.randomizeRotation = true;
    bc.minScale = 5.0f;
    bc.maxScale = 10.0f;
    bc.footprintSizeX = 6.0f;    //必要ならモデルに合わせて調整
    bc.footprintSizeZ = 6.0f;
    bc.baseColliderSize = { 3.0f, 17.0f, 3.0f };
    bc.maxAttemptsPerBuilding = 50;

    bc.fixedPositions =
    {
        { 125.0f, -12.0f,    0.0f },
        {  62.5f, -12.0f, -125.0f },
        { -62.5f, -12.0f,  125.0f },
        { -62.5f, -12.0f, -125.0f },
        {  62.5f, -12.0f,  125.0f },
        {-125.0f, -12.0f,  -62.5f },
        {-125.0f, -12.0f,   62.5f },
        {   0.0f, -12.0f,    0.0f },
    };

    int placed = m_buildingSpawner->Spawn(bc);

    //------------------スカイドーム作成-------------------------

    m_SkyDome = std::make_shared<SkyDome>("Asset/SkyDome/SkyDome_03.png");
    m_SkyDome->Initialize();

    if (m_cameraComp)
    {
        m_SkyDome->SetCamera(m_cameraComp);
    }

    //-----------------------床制作------------------------------

    auto floorObj = std::make_shared<GameObject>();
    floorObj->SetPosition(Vector3(0, -5, 0));
    floorObj->SetRotation(Vector3(0, 0, 0));
    floorObj->SetScale(Vector3(75, 75, 75));

    // AddComponent で床コンポーネントを追加（テクスチャパスは任意）
    auto floorComp = floorObj->AddComponent<FloorComponent>();

    floorComp->SetGridTexture("Asset/Texture/grid01.jpeg", 1, 1);

    floorObj->Initialize();

    AddObject(floorObj);
}

void DebugScene::InitializeUI()
{
    //-----------レティクル作成--------------
    m_reticle = std::make_shared<Reticle>(L"Asset/UI/26692699.png", m_reticleW);
    RECT rc{};
    GetClientRect(Application::GetWindow(), &rc);
    float screenWidth = static_cast<float>(rc.right - rc.left);
    float screenHeight = static_cast<float>(rc.bottom - rc.top);
    float x = screenWidth * 0.5f;
    float y = screenHeight * 0.5f;
    m_reticle->Initialize();

    //-------------HPバー作成-----------------
    auto hpUI = std::make_shared<HPBar>(L"Asset/UI/HPBar01.png", L"Asset/UI/HPGauge01.png", 100.0f, 475.0f);
    hpUI->SetScreenPos(30.0f, 200.0f);
    hpUI->Initialize();

    m_CountDown01 = std::make_shared<GameObject>();
    auto LogoTexter01 = std::make_shared<TextureComponent>();
    LogoTexter01->LoadTexture(L"Asset/UI/CountDown_01.png");
    LogoTexter01->SetSize(200.0f, 200.0f);
    LogoTexter01->SetScreenPosition(540.0f, 200.0f);
    m_CountDown01->AddComponent(LogoTexter01);

    m_CountDown02 = std::make_shared<GameObject>();
    auto LogoTexter02 = std::make_shared<TextureComponent>();
    LogoTexter02->LoadTexture(L"Asset/UI/CountDown_02.png");
    LogoTexter02->SetSize(200.0f, 200.0f);
    LogoTexter02->SetScreenPosition(540.0f, 200.0f);
    m_CountDown02->AddComponent(LogoTexter02);

    m_CountDown03 = std::make_shared<GameObject>();
    auto LogoTexter03 = std::make_shared<TextureComponent>();
    LogoTexter03->LoadTexture(L"Asset/UI/CountDown_03.png");
    LogoTexter03->SetSize(200.0f, 200.0f);
    LogoTexter03->SetScreenPosition(540.0f, 200.0f);
    m_CountDown03->AddComponent(LogoTexter03);

    m_CountDownGo = std::make_shared<GameObject>();
    auto LogoTexterGo = std::make_shared<TextureComponent>();
    LogoTexterGo->LoadTexture(L"Asset/UI/CountDown_Go.png");
    LogoTexterGo->SetSize(500.0f, 200.0f);
    LogoTexterGo->SetScreenPosition(430.0f, 200.0f);
    m_CountDownGo->AddComponent(LogoTexterGo);

    //-------------ミニマップ設定----------------
    m_miniMapBgSRV = TextureManager::Load("Asset/UI/minimap_Background.png");
    m_miniMapPlayerSRV = TextureManager::Load("Asset/UI/mimimap_player.png");
    m_miniMapEnemySRV = TextureManager::Load("Asset/UI/mimimap_enemy.png");
    m_miniMapBuildingSRV = TextureManager::Load("Asset/UI/mimimap_building.png");

    m_miniMapUi = std::make_shared<GameObject>();
    m_miniMap = m_miniMapUi->AddComponent<MiniMapComponent>().get();

    m_miniMap->SetScreenPosition(1008.0f, 16.0f);
    m_miniMap->SetSize(256.0f, 256.0f);
    m_miniMap->SetCoverageRadius(200.0f);
    m_miniMap->SetRotateWithPlayer(true);
    m_miniMap->SetIconSize(10.0f);

    m_miniMap->SetBackgroundSRV(m_miniMapBgSRV);
    m_miniMap->SetPlayerIconSRV(m_miniMapPlayerSRV);
    m_miniMap->SetEnemyIconSRV(m_miniMapEnemySRV);
    m_miniMap->SetBuildingIconSRV(m_miniMapBuildingSRV);

    m_miniMap->SetPlayer(m_player.get()); // m_playerがshared_ptr<GameObject>想定

    auto cameraComp = m_FollowCamera->GetComponent<FollowCameraComponent>();

    std::weak_ptr<HPBar> wHpUI = hpUI;

    auto hp = m_player->GetComponent<HitPointComponent>();
    if (hp)
    {
        std::weak_ptr<HitPointComponent> wPlayerHP = hp;
        std::weak_ptr<HPBar> wHpUI = hpUI;


        hp->SetOnDamaged([wHpUI, wPlayerHP, cameraComp](const DamageInfo& info)
            {
                // カメラシェイク（cameraComp が raw pointer なら null チェック）
                if (cameraComp)
                {
                    cameraComp->Shake(7.5f, 0.5f, FollowCameraComponent::ShakeMode::Horizontal);
                }

                //HPUI 更新：まず weak -> shared にする
                if (auto bar = wHpUI.lock())
                {
                    if (auto playerHP = wPlayerHP.lock())
                    {
                        // HitPointComponent 側に GetHP/GetMaxHP があれば使う
                        float cur = playerHP->GetHP();
                        float max = playerHP->GetMaxHP();
                        bar->SetHP(cur, max);
                    }
                    else
                    {
                        // 万が一 playerHP が無ければ DamageInfo に current/max が入っていれば使う
                    }
                }
            });
    }
    AddTextureObject(m_reticle);
    AddTextureObject(hpUI);
    AddTextureObject(m_miniMapUi);
}

void DebugScene::Init()
{
    LoadPlayerConfigFromIni();

    //デバッグ初期化
    InitializeDebug();

    //プレイエリア初期化
    InitializePlayArea();

    //ステート関連初期化
    InitializePhase();

    //プレイヤー初期化
    InitializePlayer();

    //カメラ初期化
    InitializeCamera();

    //敵初期化
    InitializeEnemy();

    //建物初期化
    InitializeStageObject();

    //UI初期化
    InitializeUI();

    m_GameObjects.insert(m_GameObjects.begin(), m_SkyDome);

    m_FollowCamera->GetFollowCameraComponent()->SetTarget(m_player.get());


    AddObject(m_player);
    AddObject(m_FollowCamera);

    if (m_FollowCamera && m_FollowCamera->GetFollowCameraComponent())
    {
        Vector2 screenPos(static_cast<float>(m_lastDragPos.x), static_cast<float>(m_lastDragPos.y));
        m_FollowCamera->GetFollowCameraComponent()->SetReticleScreen(screenPos);
    }

    // 例: Renderer::Init() の後
    DebugRenderer::Get().Initialize(Renderer::GetDevice(), Renderer::GetDeviceContext());
}

void DebugScene::Update(float deltatime)
{
    static float currentBlur = 0.0f;

    if (m_gameState == DebugState::Countdown)
    {
        m_countdownRemaining -= deltatime;

        m_FollowCamera->Update(deltatime);
        m_SkyDome->Update(deltatime);

        if (m_countdownRemaining <= 0.0f)
        {
            m_gameState = DebugState::Playing;
            m_countdownRemaining = 0.0f;
        }
    }

    if (m_gameState == DebugState::Playing)
    {
        if (!m_playerMove)
        {
            m_playerMove = m_player->GetComponent<MoveComponent>();
        }

		auto shootComp = m_player->GetComponent<ShootingComponent>();

        if (shootComp)
        {
			shootComp->SetBulletSpeed(m_playerBulletSpeed);
        }

        if (m_playerMove)
        {
			m_playerMove->SetSpeed(m_playerMoveSpeed);


            float boostIntensity = m_playerMove->GetBoostIntensity(); // 0～1

            float targetBlur = boostIntensity * 1.0f;

            float interpSpeed = 6.0f; //大きいほど追従が速い
            float alpha = std::min(1.0f, interpSpeed * deltatime);
            currentBlur += (targetBlur - currentBlur) * alpha;

            //---------------------------------------------------------
            //  Renderer のポストプロセス設定に反映する
            //---------------------------------------------------------
            PostProcessSettings pp = Renderer::GetPostProcessSettings();

            // ブラー強度（0～1）
            pp.motionBlurAmount = currentBlur * 3.3;

            //行列取得
            Matrix projMatrix = m_FollowCamera->GetCameraComponent()->GetProj();
            Matrix viewMatrix = m_FollowCamera->GetCameraComponent()->GetView();

            //⓪Playerの位置と注視点の位置取得
            Vector3 playerPos = m_player->GetPosition();

            std::shared_ptr<FollowCameraComponent> followCamera = m_FollowCamera->GetFollowCameraComponent();
            if (!followCamera) { return; }
            Vector3 cameraLook = followCamera->GetLookTarget();

            Vector3 playerToLook = cameraLook - playerPos;

            //②このベクトル
            // と0.0f~1.0fの間の中でどこを中心にブラーをかけるかを決定
            Vector3 blurCenter = playerPos + (cameraLook - playerPos) * 0.5f;

            //③ワールド座標→スクリーン座標変換
            XMVECTOR clip = XMVector3TransformCoord(XMLoadFloat3(&blurCenter), viewMatrix * projMatrix);

            float w = Application::GetWidth();
            float h = Application::GetHeight();

            XMMATRIX screenMat =
            {
                w / 2,   0,      0, 0,
                0,      -h / 2,  0, 0,
                0,       0,      1, 0,
                w / 2,   h / 2,  0, 1
            };

            XMVECTOR screen = XMVector3TransformCoord(clip, screenMat);

            XMFLOAT3 screenPos{};
            XMStoreFloat3(&screenPos, screen);

            float normalizedX = screenPos.x / w;
            float normalizedY = screenPos.y / h;

            pp.motionBlurCenter.x = normalizedX;
            pp.motionBlurCenter.y = normalizedY;

            //ブラーの伸びる長さ（調整ポイント）
            pp.motionBlurStretch = 0.1f;

            pp.motionBlurStart01 = 0.0f;
            pp.motionBlurEnd01 = 1.0f;

            Renderer::SetPostProcessSettings(pp);
        }

        //フレーム先頭で前フレームの登録を消す
        CollisionManager::Clear();

        //新規オブジェクトをGameSceneのオブジェクト配列に追加する
        SetSceneObject();
        //----------------- レティクルのドラッグ処理 -----------------
        if (Input::IsMouseLeftPressed())
        {
            m_isDragging = true;
        }

        if (!Input::IsMouseLeftDown() && m_isDragging)
        {
            m_isDragging = false;
        }

        //カメラにレティクル座標を渡す（最新のものを渡す）
        if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
        {
            m_FollowCamera->GetFollowCameraComponent()->SetReticleScreen(Vector2((float)m_lastDragPos.x, (float)m_lastDragPos.y));
        }

        //----------------------------------------------

        auto followCan = m_FollowCamera->GetComponent<FollowCameraComponent>();
        if (m_FollowCamera && m_reticle)
        {
            followCan->SetReticleScreen(m_reticle->GetScreenPos());
        }

        followCan->SetReticleScreen(m_reticle->camera);

        //全オブジェクト Update を一回だけ実行（重要）
        for (auto& obj : m_GameObjects)
        {
            if (obj) obj->Update(deltatime);
        }



        //全オブジェクト Update を一回だけ実行（重要）
        for (auto& obj : m_TextureObjects)
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

        //当たり判定チェック実行
        CollisionManager::CheckCollisions();

        for (auto& obj : m_GameObjects)
        {
            if (!obj) { continue; }

            auto push = obj->GetComponent<PushOutComponent>();
            if (push)
            {
                push->ApplyPush();
            }
        }

        if (m_miniMap)
        {
            std::vector<GameObject*> enemies;
            std::vector<GameObject*> buildings;

            for (std::shared_ptr<GameObject> obj : m_GameObjects)
            {
                if (!obj)
                {
                    continue;
                }

                if (auto enemy = std::dynamic_pointer_cast<Enemy>(obj))
                {
                    enemies.push_back(obj.get());
                }
                else if (auto building = std::dynamic_pointer_cast<Building>(obj))
                {
                    buildings.push_back(obj.get());
                }
            }

            m_miniMap->SetEnemies(enemies);
            m_miniMap->SetBuildings(buildings);
        }

    }

    if (Input::IsKeyDown(VK_RETURN))
    {
        SceneManager::SetCurrentScene("ResultScene");
    }
}

void DebugScene::Draw(float dt)
{
    DrawWorld(dt);
    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        auto cam = m_FollowCamera->GetCameraComponent();
        Renderer::SetViewMatrix(cam->GetView());
        Renderer::SetProjectionMatrix(cam->GetProj());
    }
}

void DebugScene::DrawWorld(float deltatime)
{
    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        auto cam = m_FollowCamera->GetCameraComponent();
        Renderer::SetViewMatrix(cam->GetView());
        Renderer::SetProjectionMatrix(cam->GetProj());
    }

    for (auto& obj : m_GameObjects)
    {
        if (!obj) { continue; }

        if (obj.get() == m_player.get()) { continue; }

        if (std::dynamic_pointer_cast<Reticle>(obj)) { continue; }
        obj->Draw(deltatime);
    }

    if (m_player)
    {
        Renderer::BeginPlayerRenderTarget();
        m_player->Draw(deltatime);
        Renderer::SetSceneRenderTarget();
    }

    if (isCollisionDebugMode)
    {
        if (m_debugRenderer && m_FollowCamera && m_FollowCamera->GetCameraComponent())
        {
            auto camComp = m_FollowCamera->GetCameraComponent();
            Matrix view = camComp->GetView();
            Matrix proj = camComp->GetProj();

            // 各オブジェクトのコライダーを登録
            for (auto& obj : m_GameObjects)
            {
                if (!obj) { continue; }
                auto col = obj->GetComponent<ColliderComponent>();
                if (!col) { continue; }

                bool hit = col->IsHitThisFrame();
                Vector4 color = hit ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 0.6f);

                Vector3 center = col->GetCenter();

                if (col->GetColliderType() == ColliderType::AABB)
                {
                    auto aabb = static_cast<AABBColliderComponent*>(col.get());
                    Vector3 mn = aabb->GetMin();
                    Vector3 mx = aabb->GetMax();
                    Vector3 size = (mx - mn);              // フルサイズ
                    m_debugRenderer->AddBox(center, size, Matrix::Identity, color);
                }
                else if (col->GetColliderType() == ColliderType::OBB)
                {
                    auto obb = static_cast<OBBColliderComponent*>(col.get());
                    Vector3 size = obb->GetSize();         // フルサイズ
                    Matrix rot = obb->GetRotationMatrix();
                    m_debugRenderer->AddBox(center, size, rot, color);
                }
                else if (col->GetColliderType() == ColliderType::SPHERE)
                {
                    auto sphere = static_cast<SphereColliderComponent*>(col.get());
                    float radius = sphere->GetRadius();
                    m_debugRenderer->AddSphere(center, radius, color, 24);
                }
            }

            // デバッグボックス描画
            m_debugRenderer->Draw(
                m_FollowCamera->GetCameraComponent()->GetView(),
                m_FollowCamera->GetCameraComponent()->GetProj()
            );
        }
    }
}

void DebugScene::DrawUI(float deltatime)
{
    if (m_gameState == DebugState::Countdown)
    {
        if (m_countdownRemaining >= 3.0f)
        {
            m_CountDown03->Draw(deltatime);
        }
        else if (m_countdownRemaining >= 2.0f)
        {
            m_CountDown02->Draw(deltatime);
        }
        else if (m_countdownRemaining >= 1.0f)
        {
            m_CountDown01->Draw(deltatime);
        }
        else if (m_countdownRemaining >= 0.0f)
        {
            m_CountDownGo->Draw(deltatime);
        }
    }

    if (m_gameState == DebugState::Playing)
    {
        for (auto& obj : m_TextureObjects)
        {
            if (!obj) { continue; }
            if (std::dynamic_pointer_cast<Reticle>(obj)) { continue; } // HUD はあとで描く
            obj->Draw(deltatime);
        }

        // HUD(レティクル)を最後に描く
        if (m_reticle)
        {
            m_reticle->Draw(deltatime);
        }

        // 旧レティクル描画が残っている場合
        if (m_reticleTex && m_reticleTex->GetSRV())
        {
            Vector2 size(m_reticleW, m_reticleH);
            Renderer::DrawReticle(m_reticleTex->GetSRV(), m_lastDragPos, size);
        }
    }
}


void DebugScene::Uninit()
{
    // ---------------- 外部登録の解除 ----------------
    CollisionManager::Clear();

    // DebugUI に「登録解除」があるならここで呼ぶ
    // DebugUI::Clear();

    // ---------------- SRVの解放（必要な設計の場合のみ） ----------------
    // TextureManager が所有しているなら Release しないこと！
    if (m_miniMapBgSRV)
    {
        m_miniMapBgSRV->Release();
        m_miniMapBgSRV = nullptr;
    }
    if (m_miniMapPlayerSRV)
    {
        m_miniMapPlayerSRV->Release();
        m_miniMapPlayerSRV = nullptr;
    }
    if (m_miniMapEnemySRV)
    {
        m_miniMapEnemySRV->Release();
        m_miniMapEnemySRV = nullptr;
    }
    if (m_miniMapBuildingSRV)
    {
        m_miniMapBuildingSRV->Release();
        m_miniMapBuildingSRV = nullptr;
    }

    // ---------------- spawner / renderer ----------------
    if (m_enemySpawner)
    {
        m_enemySpawner.reset();
    }

    if (m_buildingSpawner)
    {
        m_buildingSpawner.reset();
    }

    if (m_debugRenderer)
    {
        m_debugRenderer->Clear();
        m_debugRenderer.reset();
    }

    // ---------------- GameObject 解放 ----------------
    for (auto& obj : m_GameObjects)
    {
        if (!obj)
        {
            continue;
        }

        obj->Uninit();
        obj->SetScene(nullptr);
    }

    for (auto& obj : m_TextureObjects)
    {
        if (!obj)
        {
            continue;
        }

        obj->Uninit();
        obj->SetScene(nullptr);
    }

    m_GameObjects.clear();
    m_TextureObjects.clear();
    m_AddObjects.clear();
    m_DeleteObjects.clear();

    // ---------------- キャッシュ生ポインタは必ずnullに ----------------
    m_cameraComp = nullptr;
    m_miniMap = nullptr;

    // ---------------- shared_ptr / 参照を切る ----------------
    m_playerMove.reset();
    //m_playArea.reset();

    m_player.reset();
    m_FollowCamera.reset();
    m_SkyDome.reset();

    m_reticleObj.reset();
    m_HPObj.reset();
    m_reticleTex.reset();
    m_reticle.reset();

    // ここで enemyCount 等を初期化したいならやる
    enemyCount = 0;
    m_isDragging = false;
}


void DebugScene::AddObject(std::shared_ptr<GameObject> obj)
{
    if (!obj)
    {
        return;
    }

    //既にシーン内にいるかpendingにいるかチェック
    auto itInScene = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });

    //既に実体がある
    if (itInScene != m_GameObjects.end())
    {
        return;
    }

    auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });

    // 追加予定にすでにある
    if (itPending != m_AddObjects.end())
    {
        return;
    }

    //所属しているSceneを登録
    obj->SetScene(this);

    //実際に配列にプッシュする
    m_AddObjects.push_back(obj);
}

void DebugScene::AddTextureObject(std::shared_ptr<GameObject> obj)
{
    if (!obj)
    {
        return;
    }

    //既にシーン内にいるかpendingにいるかチェック
    auto itInScene = std::find_if(m_TextureObjects.begin(), m_TextureObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });

    //既に実体がある
    if (itInScene != m_TextureObjects.end())
    {
        return;
    }

    //所属しているSceneを登録
    obj->SetScene(this);

    //実際に配列にプッシュする
    m_TextureObjects.push_back(obj);
}

void DebugScene::RemoveObject(GameObject* obj)
{
    //ポインタがないなら処理終わり
    if (!obj) { return; }

    //------------------------------
    // コライダー登録解除
    //------------------------------
    if (auto col = obj->GetComponent<ColliderComponent>())
    {
        CollisionManager::UnregisterCollider(col.get());
    }

    //------------------------------
    // m_AddObjectsにいるか確認
    // いるなら取り消す
    //------------------------------
    auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj; });
    if (itPending != m_AddObjects.end())
    {
        m_AddObjects.erase(itPending);
        return;
    }
    //---------------------------------
    // m_GameObjectsにいるか確認
    // いるならm_DeleteObjects に登録
    // ---------------------------------
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
}

void DebugScene::FinishFrameCleanup()
{
    //m_DeleteObjectsにあるアイテムを削除
    for (auto& delSp : m_DeleteObjects)
    {
        if (!delSp) { continue; }

        //m_GameObjects から探す
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == delSp.get(); });

        if (it != m_GameObjects.end())
        {
            if (auto enemy = std::dynamic_pointer_cast<Enemy>(*it))
            {
                enemyCount -= 1;
            }

            //Uninit
            (*it)->Uninit();

            //シーン参照を切る
            (*it)->SetScene(nullptr);

            m_GameObjects.erase(it);
        }

        //m_AddObjectsにまだあるなら削除
        auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == delSp.get(); });
        if (itPending != m_AddObjects.end())
        {
            // 追加前に削除予定だったオブジェクトなら Uninit して erase
            (*itPending)->Uninit();
            (*itPending)->SetScene(nullptr);
            m_AddObjects.erase(itPending);
        }
    }

    m_DeleteObjects.clear();
}

void DebugScene::SetSceneObject()
{
    if (!m_AddObjects.empty())
    {
        //一括追加
        m_GameObjects.reserve(m_GameObjects.size() + m_AddObjects.size());
        m_GameObjects.insert(m_GameObjects.end(), std::make_move_iterator(m_AddObjects.begin()), std::make_move_iterator(m_AddObjects.end()));
        m_AddObjects.clear();
    }
}




