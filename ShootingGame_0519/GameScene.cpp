#include <iostream>

#include "GameScene.h"
#include "Input.h"
#include "renderer.h"
#include "Application.h"

#include "DebugGlobals.h" 

#include "Collision.h"
#include "CollisionManager.h"

#include "SkyDome.h"
#include "HPBar.h"
#include "Building.h"
#include "PatrolComponent.h"
#include "CircularPatrolComponent.h"
#include "FloorComponent.h"
#include "PlayAreaComponent.h"
#include "HitPointCompornent.h"
#include "PushOutComponent.h"
#include "SphereColliderComponent.h"
#include "EffectManager.h"
#include "SceneManager.h"
#include "IniFile.h"
#include "TextureManager.h"
#include "Sound.h"

#include "CsvGridLoader.h"

/// <summary>
/// ファイルが存在しているかどうかを探す関数
/// </summary>
/// <param name="path">ファイルパス</param>
/// <returns></returns>
static bool IsFileExists(const std::string& path)
{
    DWORD attr = GetFileAttributesA(path.c_str());

    //ファイルが存在しないか、ディレクトリだった場合は失敗
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    //ファイルがディレクトリだった場合は失敗
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }

    return true;
}

/// <summary>
/// PlayerやCameraの設定を.iniファイルから読み込む為の関数
/// </summary>
bool GameScene::LoadPlayerConfigFromIni()
{
    if (!IsFileExists(m_iniPath))
    {
        return false;
    }

    IniFile ini(m_iniPath.c_str());

    //読み込み
    m_playerMoveSpeed = ini.ReadFloat("Player", "MoveSpeed", m_playerMoveSpeed);
    m_playerBoostMultiplier = ini.ReadFloat("Player", "BoostMultiplier", m_playerBoostMultiplier);
    m_playerBulletSpeed = ini.ReadFloat("Player", "BulletSpeed", m_playerBulletSpeed);
    m_playerHp = ini.ReadFloat("Player", "HpMax", m_playerHp);

    //読み込み
    m_cameraDistance = ini.ReadFloat("Camera", "Distance", m_cameraDistance);
    m_cameraHeight = ini.ReadFloat("Camera", "Height", m_cameraHeight);
    m_cameraFovDeg = ini.ReadFloat("Camera", "FovDeg", m_cameraFovDeg);
    m_cameraBoostFovDeg = ini.ReadFloat("Camera", "BoostFovDeg", m_cameraBoostFovDeg);
    m_cameraSensitivity = ini.ReadFloat("Camera", "Sensitivity", m_cameraSensitivity);

    //読み込み
    m_blurStretch = ini.ReadFloat("Blur", "Stretch", m_blurStretch);
    m_blurStartPoint = ini.ReadFloat("Blur", "StartPoint", m_blurStartPoint);
    m_blurEndPoint = ini.ReadFloat("Blur", "EndPoint", m_blurEndPoint);
    m_blurCenterX = ini.ReadFloat("Blur", "CenterX", m_blurCenterX);
    m_blurCenterY = ini.ReadFloat("Blur", "CenterY", m_blurCenterY);

    return true;
}

void GameScene::DebugCollisionMode()
{
    static int selected = 1;

    ImGui::Begin("Collision Debug Mode Select");

    ImGui::RadioButton("OnDebug Mode", &selected, 0);
    ImGui::RadioButton("NoDebug Mode", &selected, 1);

    ImGui::End();

    if (selected == 0)
    {
        isCollisionDebugMode = true;
    }
    else
    {
        isCollisionDebugMode = false;
    }
}

void GameScene::DebugGameDateSet()
{

}

bool GameScene::Raycast(const Vector3& origin,
    const Vector3& dir,
    float maxDistance,
    RaycastHit& outHit,
    std::function<bool(GameObject*)> predicate,
    GameObject* ignore)
{
    return CollisionManager::RaycastWorld(origin, dir, maxDistance, outHit, predicate, ignore);
}

/// <summary>
/// IMGUI関連やデバッグ描画関連の初期化を行う関数
/// </summary>
void GameScene::InitializeDebug()
{
    //DebugUI::RedistDebugFunction([this]() {DebugSetPlayerSpeed(); });
    //DebugUI::RedistDebugFunction([this]() {DebugSetAimDistance(); });
	//DebugUI::RedistDebugFunction([this]() {DebugMotionBlur(); });

    //DebugRendererの初期化
    m_debugRenderer = std::make_unique<DebugRenderer>();
    m_debugRenderer->Initialize(Renderer::GetDevice(), 
                                Renderer::GetDeviceContext(),
                                L"DebugLineVS.cso", L"DebugLinePS.cso");
}

/// <summary>
/// Playerの動ける範囲などを設定する関数
/// </summary>
void GameScene::InitializePlayArea()
{
    m_playArea = std::make_shared<PlayAreaComponent>();
    m_playArea->SetScene(this); 
    m_playArea->SetBounds({ -300.0f, -1.0f, -300.0f }, { 300.0f, 80.0f, 300.0f });
    m_playArea->SetGroundY(-7.0f);
}

/// <summary>
/// ステージの状態を初期化する関数
/// </summary>
void GameScene::InitializePhase()
{
    m_gameState = GameState::Countdown;
    m_countdownRemaining = 4.0f;
}

/// <summary>
/// 
/// </summary>
void GameScene::InitializeCamera()
{
    m_FollowCamera = std::make_shared<CameraObject>();
    m_cameraComp = m_FollowCamera->AddCameraComponent<FollowCameraComponent>().get();
    //AddObject(m_FollowCamera);
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
        m_cameraComp->SetPlayArea(m_playArea.get());
    }

    if (m_FollowCamera)
    {
        auto followCom = m_FollowCamera->GetComponent<FollowCameraComponent>();
        if (followCom)
        {
            followCom->SetFov(m_cameraFovDeg);
            followCom->SetBoostFov(m_cameraBoostFovDeg);
            followCom->SetSensitivity(m_cameraSensitivity);
            followCom->SetDistance(m_cameraDistance);
            followCom->SetHeight(m_cameraHeight);
        }
    }

    Vector3 Ppos = m_player->GetPosition();
    m_FollowCamera->SetPosition({ Ppos.x, Ppos.y, Ppos.z - 20 });
}

void GameScene::InitializePlayer()
{
    m_player = std::make_shared<Player>();
    m_player->Initialize();

    if (auto moveComp = m_player->GetComponent<MoveComponent>())
    {
        moveComp->SetPlayArea(m_playArea.get());

        moveComp->SetSpeed(m_playerMoveSpeed);
        moveComp->SetBoostMultiplier(m_playerBoostMultiplier);
    }

    if (auto shootComp = m_player->GetComponent<ShootingComponent>())
    {
        shootComp->SetScene(this);

        shootComp->SetBulletSpeed(m_playerBulletSpeed);
    }

    if (auto hpComp = m_player->GetComponent<HitPointComponent>())
    {
         hpComp->SetMaxHP(m_playerHp);
    }

    AddObject(m_player);
}

void GameScene::InitializeEnemy()
{
    m_enemySpawner = std::make_unique<EnemySpawner>(this);
    m_enemySpawner->SetOnPatrolEnemyDefeated([this](Enemy* enemy)
        {
            m_enemyKillCount += 1;

            char buf[128];
            sprintf_s(buf, "PatrolEnemy Count = %d / %d\n",
                m_enemyKillCount,
                m_clearKillCount);
            OutputDebugStringA(buf);
        });
    m_enemySpawner->patrolCfg.spawnCount = m_keepPatrolEnemyCount;
    m_enemySpawner->circleCfg.spawnCount = 0;
    m_enemySpawner->turretCfg.spawnCount = 2;

    enemyCount = m_enemySpawner->patrolCfg.spawnCount + m_enemySpawner->circleCfg.spawnCount + m_enemySpawner->turretCfg.spawnCount;

    //========================
    // Waypoint Set 0
    //========================
    m_enemySpawner->SetWaypoints(
    {
        { -101.363f, 15.0f, -172.409f },
        { -47.9751f, 15.0f, -84.748f },
        {  16.7131f, 15.0f, -119.96f },
        {  68.0036f, 15.0f, -110.977f },
        { 117.093f,  15.0f, -13.24f },

        { 114.194f, 15.0f,  35.7788f },
        {  85.8455f, 15.0f, 73.4013f },
        {  35.522f,  15.0f, 68.8446f },

        { -52.807f, 15.0f, 83.2626f },
        { -76.0765f, 15.0f, 126.96f },

        { -22.8005f, 15.0f, 196.188f },
        {  39.4108f, 15.0f, 232.437f },
        { 111.776f, 15.0f, 255.559f },

        { 149.03f, 15.0f, 230.977f },
    });


    m_enemySpawner->SetWaypoints(
        {
            { -118.601f, 35.0f, 168.216f },
            {  5.91842f, 35.0f, 227.306f },
            {  73.4986f, 35.0f, 268.295f },

            {  137.683f, 35.0f, 237.167f },
            {  133.894f, 35.0f, 202.164f },

            {   93.4445f, 35.0f, 68.7327f },
            {  117.852f, 35.0f, -9.21532f },

            {  135.603f, 35.0f, -92.545f },
            {  102.306f, 35.0f, -135.472f },

            {   33.1515f, 35.0f, -165.786f },
            {  -57.6906f, 35.0f, -187.063f },

            { -184.946f, 35.0f, -158.122f },
            { -142.524f, 35.0f, -76.5417f },

            { -130.316f, 35.0f, 65.0955f },
            { -141.164f, 35.0f, 175.543f },
        });

    m_enemySpawner->SetWaypoints(
        {
            {   47.6f,  12.1f, -246.0f },
            {    3.6f,   7.6f, -121.5f },
            {  -31.9f,   3.9f,  -91.0f },
            {  -69.6f,  -1.0f,  -44.2f },
            {  -32.8f,  -1.0f,  -30.9f },

            {  -11.4f,  -1.0f,  -74.7f },
            {   35.2f,   0.0f,  -81.9f },
            {   82.9f,   3.8f,  -57.2f },
            {  106.5f,   6.8f,  -27.4f },
            {  113.6f,   7.2f,   17.5f },

            {  125.8f,   5.3f,   57.5f },
            {  166.0f,   3.0f,   88.4f },
            {  215.7f,  -0.6f,  163.1f },
            {  178.4f,  -0.8f,  208.8f },
            {  142.3f,   2.6f,  243.8f },

            {  100.8f,  19.1f,  261.7f },
            {   63.6f,  28.0f,  259.8f },
            {   -6.1f,   0.8f,  206.6f },
            {  -56.1f,   6.6f,  165.0f },
            {  -42.0f,   6.4f,  -13.0f },

            {  -68.9f,   0.6f,  -55.2f },
            {  -93.6f,  14.2f, -113.9f },
            {  -80.7f,  32.4f, -163.3f },
            {  -26.9f,  29.1f, -193.2f },
            {   64.2f,  25.3f, -181.4f },
        });

    m_enemySpawner->SetWaypoints(
        {
            { -405.0f, 90.0f, -495.0f },
            {  405.0f, 90.0f,  495.0f },
            {  405.0f, 90.0f, -495.0f },
            { -405.0f, 90.0f,  495.0f },
        });

    // 生成
    m_enemySpawner->EnsurePatrolCount();

    // Turret
    m_enemySpawner->turretCfg.target = m_player;
    m_enemySpawner->turretCfg.bulletSpeed = 100.0f;
    m_enemySpawner->SetTurretPos({ 100.0f,100.0f,0.0f });
    m_enemySpawner->SetTurretPos({ -100.0f,100.0f,0.0f });
    m_enemySpawner->EnsureTurretCount();
}

void GameScene::InitializeStageObject()
{
    m_buildingSpawner = std::make_unique<BuildingSpawner>(this);

    //-----------------------
    // CSVから建物配置を読み込み
    //-----------------------
    auto grid = CsvGridLoader::LoadGrid("Data/FreeStage01.csv");

    if (!grid.empty())
    {
        float cellSize = 45.0f;

        int rowCount = static_cast<int>(grid.size());
        int colCount = static_cast<int>(grid[0].size());

        int centerCol = colCount / 2;
        int centerRow = rowCount / 2;

        std::vector<Vector3> type1Positions;
        std::vector<Vector3> type2Positions;

        for (int row = 0; row < rowCount; row++)
        {
            for (int col = 0; col < static_cast<int>(grid[row].size()); col++)
            {
                int value = grid[row][col];

                float x = static_cast<float>(col - centerCol) * cellSize;
                float y = -12.0f;
                float z = static_cast<float>(row - centerRow) * cellSize;

                if (value == 1)
                {
                    type1Positions.push_back(Vector3(x, y, z));
                }
                else if (value == 2)
                {
                    type2Positions.push_back(Vector3(x, y, z));
                }
            }
        }

        //-----------------------
        // 1番の岩
        //-----------------------
        BuildingConfig bc1;
        bc1.modelPath = "Asset/Build/rock_0817055319_refine.obj";
        bc1.count = static_cast<int>(type1Positions.size());
        bc1.fixedPositions = type1Positions;
        bc1.spacing = 30.0f;

        bc1.scaleX = 60.0f;
        bc1.scaleY = 80.0f;
        bc1.scaleZ = 60.0f;

        bc1.baseColliderSize = { 1.0f, 1.0f, 1.0f };
        bc1.maxAttemptsPerBuilding = 50;

        m_buildingSpawner->Spawn(bc1);

        //-----------------------
        // 2番の岩 大きめ
        //-----------------------
        BuildingConfig bc2;
        bc2.modelPath = "Asset/Build/rock02/rock_0817054342_refine.obj";
        bc2.count = static_cast<int>(type2Positions.size());
        bc2.fixedPositions = type2Positions;
        bc2.spacing = 40.0f;

        bc2.scaleX = 80.0f;
        bc2.scaleY = 200.0f;
        bc2.scaleZ = 80.0f;

        bc2.baseColliderSize = { 1.0f, 1.0f, 1.0f };
        bc2.maxAttemptsPerBuilding = 50;

        m_buildingSpawner->Spawn(bc2);


    }

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

    auto floorComp = floorObj->AddComponent<FloorComponent>();
    floorComp->SetGridTexture("Asset/Texture/grid01.jpeg", 1, 1);

    floorObj->Initialize();

    AddObject(floorObj);
}

void GameScene::InitializeUI()
{
    //-----------レティクル作成--------------
    m_reticle = std::make_shared<Reticle>(L"Asset/UI/26692699.png", m_reticleW);
    m_reticle->Initialize();

    //-------------HPバー作成-----------------
    auto hpUI = std::make_shared<HPBar>(L"Asset/UI/HPBar01.png", L"Asset/UI/HPGauge01.png", 100.0f, 475.0f);
    hpUI->Initialize();
	hpUI->SetScreenPos(16.0f, 200.0f);

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

    m_killLabelTexture = std::make_shared<GameObject>();

    auto LogoKillTexter = std::make_shared<TextureComponent>();
    LogoKillTexter->LoadTexture(L"Asset/UI/Gekihasu.png");
    LogoKillTexter->SetSize(150.0f, 60.0f);
    LogoKillTexter->SetScreenPosition(20.0f, 20.0f);

    m_killLabelTexture->AddComponent(LogoKillTexter);
    m_killLabelTexture->Initialize();

    AddTextureObject(m_killLabelTexture);

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

    m_KillCountNumberUI.LoadDigitTextures("Asset/UI/Number");
    m_KillCountNumberUI.SetPosition({ 40.0f, 90.0f });
    m_KillCountNumberUI.SetDigitSize({ 60.0f, 72.0f });
    m_KillCountNumberUI.SetSpacing(2.0f);

    m_ClearCountNumberUI.LoadDigitTextures("Asset/UI/Number");
    m_ClearCountNumberUI.SetPosition({ 180.0f, 90.0f });
    m_ClearCountNumberUI.SetDigitSize({ 60.0f, 72.0f });
    m_ClearCountNumberUI.SetSpacing(2.0f);

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

void GameScene::InitializeEffect()
{
    pp.motionBlurCenter = { m_blurCenterX, m_blurCenterY };
    pp.motionBlurStart01 = m_blurStartPoint;
    pp.motionBlurEnd01 = m_blurEndPoint;
    pp.motionBlurStretch = m_blurStretch;
}

void GameScene::InitializeWingTrail()
{
   
}

void GameScene::Init()
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
    InitializeEffect();

    InitializeWingTrail();
    m_GameObjects.insert(m_GameObjects.begin(), m_SkyDome);

    m_FollowCamera->GetFollowCameraComponent()->SetTarget(m_player.get());

    AddObject(m_FollowCamera);

    if (m_FollowCamera && m_FollowCamera->GetFollowCameraComponent())
    {
        Vector2 screenPos(static_cast<float>(m_lastDragPos.x), static_cast<float>(m_lastDragPos.y));
        m_FollowCamera->GetFollowCameraComponent()->SetReticleScreen(screenPos);
    }

    // 例: Renderer::Init() の後
    DebugRenderer::Get().Initialize(Renderer::GetDevice(), Renderer::GetDeviceContext());
}

void GameScene::Update(float deltatime)
{
    DebugGameDateSet();

    static float currentBlur = 0.0f;

    if (m_gameState == GameState::Countdown)
    {
        if (m_countdownRemaining >= 4.0f)
        {
            Sound::PlaySeWav(L"Asset/Sound/SE/Countdown_SE.wav", 0.3f);
        }

        m_countdownRemaining -= deltatime;

        m_FollowCamera->Update(deltatime);
        m_SkyDome->Update(deltatime);

        if (m_countdownRemaining <= 0.0f)
        {
            m_gameState = GameState::Playing;
            m_countdownRemaining = 0.0f;
            Sound::PlayBgmWav(L"Asset/Sound/BGM/StageBGM.wav", 0.1f);
        }
    }

    if (m_gameState == GameState::Playing)
    {
        //DebugCollisionMode();

        if (!m_playerMove)
        {
            m_playerMove = m_player->GetComponent<MoveComponent>();
        }

        if (m_playerMove)
        {
            float boostIntensity = m_playerMove->GetBoostIntensity();

            float targetBlur = boostIntensity * 1.0f;

            float interpSpeed = 6.0f;
            float alpha = std::min(1.0f, interpSpeed * deltatime);
            currentBlur += (targetBlur - currentBlur) * alpha;

            PostProcessSettings pp = Renderer::GetPostProcessSettings();
            pp.motionBlurAmount = currentBlur * 5.3f;

            Matrix projMatrix = m_FollowCamera->GetCameraComponent()->GetProj();
            Matrix viewMatrix = m_FollowCamera->GetCameraComponent()->GetView();

            Vector3 playerPos = m_player->GetPosition();

            std::shared_ptr<FollowCameraComponent> followCamera = m_FollowCamera->GetFollowCameraComponent();
            if (!followCamera){ return; }

            Vector3 cameraLook = followCamera->GetLookTarget();
            Vector3 blurCenter = playerPos + (cameraLook - playerPos) * 0.8f;

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

            pp.motionBlurCenter.x = screenPos.x / w;
            pp.motionBlurCenter.y = screenPos.y / h;
            pp.motionBlurStretch = 0.5f;
            pp.motionBlurStart01 = 0.2f;
            pp.motionBlurEnd01 = 1.0f;

            Renderer::SetPostProcessSettings(pp);
        }

        CollisionManager::Clear();

        //----------------- レティクルのドラッグ処理 -----------------
        if (Input::IsMouseLeftPressed())
        {
            m_isDragging = true;
        }

        if (!Input::IsMouseLeftDown() && m_isDragging)
        {
            m_isDragging = false;
        }

        if (m_FollowCamera && m_reticle)
        {
            if (auto followCan = m_FollowCamera->GetComponent<FollowCameraComponent>())
            {
                followCan->SetReticleScreen(m_reticle->GetScreenPos());
            }
        }

        //----------------- 既存オブジェクト更新 -----------------
        for (auto& obj : m_GameObjects)
        {
            if (!obj){ continue; }

            if (!obj->GetIsActive()){ continue; }

            obj->Update(deltatime);
        }

        for (auto& obj : m_TextureObjects)
        {
            if (!obj){ continue; }

            if (!obj->GetIsActive()) { continue; }

            obj->Update(deltatime);
        }

        //----------------- コライダー登録 -----------------
        for (auto& obj : m_GameObjects)
        {
            if (!obj) { continue; }

            if (!obj->GetIsActive()) { continue; }

            auto collider = obj->GetComponent<ColliderComponent>();
            if (collider)
            {
                CollisionManager::RegisterCollider(collider.get());
            }
        }

        //----------------- 当たり判定 -----------------
        CollisionManager::CheckCollisions();

        //----------------- 押し出し -----------------
        for (auto& obj : m_GameObjects)
        {
            if (!obj){ continue; }

            if (!obj->GetIsActive()) { continue; }

            auto push = obj->GetComponent<PushOutComponent>();
            if (push)
            {
                push->ApplyPush();
            }
        }

        //----------------- 削除処理を実行 -----------------
        FinishFrameCleanup();

        //----------------- 必要数を維持するよう補充 -----------------
        if (m_enemySpawner)
        {
            if (m_enemyKillCount < m_clearKillCount)
            {
                m_enemySpawner->patrolCfg.spawnCount = m_keepPatrolEnemyCount;
                m_enemySpawner->EnsurePatrolCount();
            }
        }

        //----------------- 追加予約を実際に反映 -----------------
        SetSceneObject();

        //----------------- ミニマップ更新 -----------------
        if (m_miniMap)
        {
            std::vector<GameObject*> enemies;
            std::vector<GameObject*> buildings;

            for (std::shared_ptr<GameObject> obj : m_GameObjects)
            {
                if (!obj){ continue; }

                if (!obj->GetIsActive()){ continue; }

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

    auto hp = m_player->GetComponent<HitPointComponent>();

    if (m_enemyKillCount >= m_clearKillCount)
    {
        SceneManager::SetCurrentScene("ResultScene");
    }

    if (hp && hp->GetHP() <= 0)
    {
        SceneManager::SetCurrentScene("ResultScene02");
    }

    static bool qPrev = false;

    bool qNow = (GetAsyncKeyState('Q') & 0x8000) != 0;

    if (qNow && !qPrev)
    {
        if (m_player)
        {
            Vector3 pos = m_player->GetPosition();

            std::cout
                << "{ "
                << pos.x << "f, "
                << pos.y << "f, "
                << pos.z << "f },"
                << std::endl;
        }
    }

    qPrev = qNow;
}

void GameScene::UpdateBuildingOcclusionFade()
{
    if (!m_player){ return; }

    if (!m_cameraComp){ return; }

    Vector3 playerPos = m_player->GetPosition();
    Vector3 cameraPos = m_cameraComp->GetOwner()->GetPosition();

    Vector3 rayVec = playerPos - cameraPos;
    float rayLength = rayVec.Length();

    if (rayLength <= 0.001f){ return; }

    Vector3 rayDir = rayVec;
    rayDir.Normalize();

    for (auto& weakBuilding : m_stageBuildings)
    {
        auto building = weakBuilding.lock();

        if (!building){ continue; }

        building->SetAlpha(1.0f);

        bool isHit = false;

        auto aabb = building->GetComponent<AABBColliderComponent>();

        if (aabb)
        {
            float hitDistance = 0.0f;
            Vector3 hitNormal = Vector3::Zero;

            isHit = Collision::RayVsAABB(
                cameraPos,
                rayDir,
                rayLength,
                aabb->GetMin(),
                aabb->GetMax(),
                hitDistance,
                hitNormal);
        }

        auto sphere = building->GetComponent<SphereColliderComponent>();

        if (!isHit && sphere)
        {
            float hitDistance = 0.0f;
            Vector3 hitNormal = Vector3::Zero;

            isHit = Collision::RayVsSphere(
                cameraPos,
                rayDir,
                rayLength,
                sphere->GetCenter(),
                sphere->GetRadius(),
                hitDistance,
                hitNormal);
        }

        if (isHit)
        {
            building->SetAlpha(0.25f);
        }
    }
}

void GameScene::Draw(float dt)
{
    DrawWorld(dt);
    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        auto cam = m_FollowCamera->GetCameraComponent();
        Renderer::SetViewMatrix(cam->GetView());
        Renderer::SetProjectionMatrix(cam->GetProj());
    }
}

void GameScene::DrawWorld(float deltatime)
{
    if (m_FollowCamera && m_FollowCamera->GetCameraComponent())
    {
        auto cam = m_FollowCamera->GetCameraComponent();
        Renderer::SetViewMatrix(cam->GetView());
        Renderer::SetProjectionMatrix(cam->GetProj());
    }

    for (auto& obj : m_GameObjects)
    {
        if (!obj){ continue; }

        if (!obj->GetIsActive()){ continue; }

        if (obj.get() == m_player.get()){ continue; }

        if (std::dynamic_pointer_cast<Reticle>(obj)){ continue; }

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
                if (!obj){ continue; }

                if (!obj->GetIsActive()){ continue; }

                auto col = obj->GetComponent<ColliderComponent>();
                
                if (!col){ continue; }

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

void GameScene::DrawUI(float deltatime)
{
    if (m_gameState == GameState::Countdown)
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

    if (m_gameState == GameState::Playing)
    {
        for (auto& obj : m_TextureObjects)
        {
            if (!obj){ continue; }

            if (!obj->GetIsActive()){ continue; }

            if (std::dynamic_pointer_cast<Reticle>(obj)){ continue; }

            obj->Draw(deltatime);
        }

        m_KillCountNumberUI.DrawNumber(m_enemyKillCount);
        m_ClearCountNumberUI.DrawNumber(m_clearKillCount);

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


void GameScene::Uninit()
{
    // ---------------- 外部登録の解除 ----------------
    CollisionManager::Clear();

    // DebugUI に「登録解除」があるならここで呼ぶ
    // DebugUI::Clear();

    // ---------------- SRVの解放（必要な設計の場合のみ） ----------------
    // TextureManager が所有しているなら Release しないこと！
   /* if (m_miniMapBgSRV)
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
    }*/

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
        if (!obj){ continue; }

        obj->Uninit();
        obj->SetScene(nullptr);
    }

    for (auto& obj : m_TextureObjects)
    {
        if (!obj) { continue; }

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
    m_playArea.reset();

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

void GameScene::AddStageBuilding(const std::shared_ptr<Building>& building)
{
    if (!building){ return; }

    m_stageBuildings.push_back(building);
}

void GameScene::AddObject(std::shared_ptr<GameObject> obj)
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

void GameScene::AddTextureObject(std::shared_ptr<GameObject> obj)
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

void GameScene::RemoveObject(GameObject* obj)
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

void GameScene::FinishFrameCleanup()
{
    for (auto& delSp : m_DeleteObjects)
    {
        if (!delSp){ continue; }

        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == delSp.get(); });

        if (it != m_GameObjects.end())
        {
            (*it)->Uninit();
            (*it)->SetScene(nullptr);

            m_GameObjects.erase(it);
        }

        auto itPending = std::find_if(m_AddObjects.begin(), m_AddObjects.end(),
            [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == delSp.get(); });

        if (itPending != m_AddObjects.end())
        {
            (*itPending)->Uninit();
            (*itPending)->SetScene(nullptr);
            m_AddObjects.erase(itPending);
        }
    }

    m_DeleteObjects.clear();

}

void GameScene::SetSceneObject()
{ 
    if (!m_AddObjects.empty())
    { 
        //一括追加
        m_GameObjects.reserve(m_GameObjects.size() + m_AddObjects.size()); 
        m_GameObjects.insert(m_GameObjects.end(), std::make_move_iterator(m_AddObjects.begin()), std::make_move_iterator(m_AddObjects.end())); 
        m_AddObjects.clear();
    } 
}

void GameScene::AddEnemyKillCount(int value)
{
    m_enemyKillCount += value;

    if (m_enemyKillCount >= m_clearKillCount)
    {
        m_enemyKillCount = m_clearKillCount;
    }
}

void GameScene::AddDefeatedPatrolEnemyCount()
{
    m_clearKillCount += 1;

    char buf[128];
    sprintf_s(buf, "PatrolEnemy Count = %d / %d\n",
        m_clearKillCount,
        m_clearKillCount);
    OutputDebugStringA(buf);
}
