#include <iostream>
#include <algorithm>
#include "TitleScene.h"
#include "Input.h"
#include "Player.h"
#include "TitleBackGround.h"
#include "Input.h"
#include "TransitionManager.h"
#include "FreeCameraComponent.h"
#include "ModelComponent.h"
#include "TextureComponent.h"
#include "Sound.h"
#include "EffectManager.h"
#include "SceneManager.h"

void TitleScene::Init()
{
    m_camera = std::make_shared<CameraObject>();
    auto freeCamComp = std::make_shared<FreeCameraComponent>();
    m_camera->AddComponent(freeCamComp);

    m_player = std::make_shared<GameObject>();
    m_player->SetPosition(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 40.0f));
    m_player->SetScale(DirectX::SimpleMath::Vector3(1.5f, 1.5f, 1.5f));

    auto model = std::make_shared<ModelComponent>("Asset/Model/Player/Fighterjet.obj");
    model->SetColor(Color(1, 0, 0, 1));
    m_player->AddComponent(model);

    m_titleLogo = std::make_shared<GameObject>();
    auto logoTexter = std::make_shared<TextureComponent>();
    logoTexter->LoadTexture(L"Asset/UI/TitleLogo01.png");
    logoTexter->SetSize(614.4f, 520.8f);
    logoTexter->SetScreenPosition(340, -30);
    m_titleLogo->AddComponent(logoTexter);

    m_titleText = std::make_shared<GameObject>();
    auto textTexter = std::make_shared<TextureComponent>();
    textTexter->LoadTexture(L"Asset/UI/TitleText02.png");
    textTexter->SetSize(780, 260);
    textTexter->SetScreenPosition(250, 370);
    m_titleText->AddComponent(textTexter);

    auto motion = std::make_shared<TitlePlayerMotionComponent>();

    m_player->AddComponent(motion);

    m_titleMotion = m_player->GetComponent<TitlePlayerMotionComponent>();

    SetupPlayerPaths();
    ApplyCurrentPlayerPath();

    m_skyDome = std::make_shared<SkyDome>("Asset/SkyDome/SkyDome_03.png");
	m_skyDome->Initialize();

	AddObject(m_skyDome);
	AddObject(m_camera);
    //cameraObj->SetCameraComponent(freeCamComp);
    AddObject(m_player);

    SetSceneObject();

    for (auto& obj : m_gameObjects)
    {
        if (obj)
        {
            obj->Initialize();
        }
    }

    if (m_skyDome && freeCamComp)
    {
        m_skyDome->SetCamera(freeCamComp.get());
    }

    Sound::PlaySeWav(L"Asset/Sound/SE/TitlePlayerPassing.wav", 0.5f);
}

void TitleScene::Update(float deltatime)
{
    SetSceneObject();

    for (auto& obj : m_gameObjects)
    {
        if (obj)
        {
            obj->Update(deltatime);
        }
    }

    for (auto& obj : m_textureObjects)
    {
        if (obj)
        {
            obj->Update(deltatime);
        }
    }

    //--------------最初の1回だけ：ロゴ表示＆入力解禁------------------
    if (!m_isLogoShown && m_titleMotion)
    {
        float safeDuration = max(0.01f, m_titleMotion->GetDuration());
        float t = std::clamp(m_titleMotion->GetTime() / safeDuration, 0.0f, 1.0f);

        if (t >= 0.7f)
        {
            Sound::PlayBgmWav(L"Asset/Sound/BGM/TitleBGM.wav", 0.1f);
            Sound::StopAllSe();

            m_isLogoShown = true;

            AddTextureObject(m_titleLogo);
            AddTextureObject(m_titleText);

            if (m_titleLogo)
            {
                m_titleLogo->Initialize();
            }
            if (m_titleText)
            {
                m_titleText->Initialize();
            }
        }
    }

	//次のPlayerのベジェ曲線での動きへ進行
    if (m_titleMotion)
    {
        float safeDuration = max(0.01f, m_titleMotion->GetDuration());
        float t = std::clamp(m_titleMotion->GetTime() / safeDuration, 0.0f, 1.0f);

        if (t >= 1.0f)
        {
            AdvancePlayerPath();
        }
    }

	//Logo表示後の点滅＆Enterキーでシーン遷移
    if (m_isLogoShown)
    {
        m_blinkTimer += deltatime;

        if (m_blinkTimer >= m_blinkInterval)
        {
            m_blinkTimer -= m_blinkInterval;
            m_blinkVisible = !m_blinkVisible;

            if (m_titleText)
            {
                auto tex = m_titleText->GetComponent<TextureComponent>();
                if (tex)
                {
                    tex->SetVisible(m_blinkVisible);
                }
            }
        }

        if (Input::IsKeyDown(VK_RETURN))
        {
            if (TransitionManager::IsTransitioning()){ return; }

            Sound::PlaySeWav(L"Asset/Sound/SE/TitleSelect01.wav", 0.5f);

            TransitionManager::Start(3.0f,
                []()
                {
                    SceneManager::SetCurrentScene("GameScene");
                });
        }

        if (Input::IsKeyDown('D'))
        {
            if (TransitionManager::IsTransitioning()) { return; }

            Sound::PlaySeWav(L"Asset/Sound/SE/TitleSelect01.wav", 0.5f);

            TransitionManager::Start(3.0f,
                []()
                {
                    SceneManager::SetCurrentScene("GameForwardScene");
                });
        }
    }
}

void TitleScene::Draw(float dt)
{
    //DrawWorld(dt);
    //Renderer::ApplyMotionBlur();
    //DrawUI(dt);
}

void TitleScene::DrawWorld(float deltatime)
{
    auto freecam = m_camera->GetComponent<FreeCameraComponent>();

    if (freecam)
    {
        Renderer::SetViewMatrix(freecam->GetView());
        Renderer::SetProjectionMatrix(freecam->GetProj());
    }

    for (auto& obj : m_gameObjects)
    {
        if (!obj) { continue; }
        obj->Draw(deltatime);
    }
}

void TitleScene::DrawUI(float deltatime)
{
    for (auto& obj : m_textureObjects)
    {
        if (!obj) { continue; }
        obj->Draw(deltatime);
    }
}

void TitleScene::Uninit()
{

}


void TitleScene::AddObject(std::shared_ptr<GameObject> obj)
{
    if (!obj)
    {
        return;
    }

    //既にシーン内にいるかpendingにいるかチェック
    auto itInScene = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });

    //既に実体がある
    if (itInScene != m_gameObjects.end())
    {
        return;
    }

    auto itPending = std::find_if(m_addObjects.begin(), m_addObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });

    // 追加予定にすでにある
    if (itPending != m_addObjects.end())
    {
        return;
    }

    //所属しているSceneを登録
    obj->SetScene(this);

    //実際に配列にプッシュする
    m_addObjects.push_back(obj);
}

void TitleScene::AddTextureObject(std::shared_ptr<GameObject> obj)
{
    if (!obj)
    {
        return;
    }

    //既にシーン内にいるかpendingにいるかチェック
    auto itInScene = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [&](const std::shared_ptr<GameObject>& sp) { return sp.get() == obj.get(); });

    //既に実体がある
    if (itInScene != m_gameObjects.end())
    {
        return;
    }

    //所属しているSceneを登録
    obj->SetScene(this);

    //実際に配列にプッシュする
    m_textureObjects.push_back(obj);
}


void TitleScene::RemoveObject(std::shared_ptr<GameObject> obj)
{
    m_deleteObjects.push_back(obj);
}

void TitleScene::RemoveObject(GameObject* obj)
{
    //if (!obj) return;
    //// 重複追加を防ぎたい場合チェックしてから push_back してもよい
    //m_deleteObjects.push_back(obj);
}

/// <summary>
/// シーンのフレーム終了後のクリーンアップ処理関数
/// </summary>
void TitleScene::FinishFrameCleanup()
{
    for (auto& p : m_deleteObjects) // p は shared_ptr<GameObject>
    {
        auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&](const std::shared_ptr<GameObject>& sp)
            {return sp == p; });
        if (it != m_gameObjects.end()) m_gameObjects.erase(it);
    }
    m_deleteObjects.clear();
}

/// <summary>
/// シーンのオブジェクトをセットする関数
/// </summary>
void TitleScene::SetSceneObject()
{
    if (!m_addObjects.empty())
    {
        // 一括追加（file-safe: reserve してから insert）
        m_gameObjects.reserve(m_gameObjects.size() + m_addObjects.size());
        m_gameObjects.insert(m_gameObjects.end(),
            std::make_move_iterator(m_addObjects.begin()),
            std::make_move_iterator(m_addObjects.end()));
        m_addObjects.clear();
    }
}

/// <summary>
/// ベジェ曲線で複数の動きを作るためにパスを設定する関数
/// </summary>
void TitleScene::SetupPlayerPaths()
{
    m_playerPaths.clear();

    //最初の動き(画面の奥から迫ってくる)
    BezierPath a;
    a.p0 = DirectX::SimpleMath::Vector3(-166.0f, 90.0f, -220.0f);
    a.p1 = DirectX::SimpleMath::Vector3(-70.0f, 12.0f, -80.0f);
    a.p2 = DirectX::SimpleMath::Vector3(10.0f, -6.0f, -3.0f);  // 近くを横切る
    a.p3 = DirectX::SimpleMath::Vector3(45.0f, -14.0f, 55.0f);  // 背面へ
    a.duration = 2.2f;
    m_playerPaths.push_back(a);

	//高い所からゆっくり横切る
    BezierPath b;
    b.p0 = DirectX::SimpleMath::Vector3(42.0f, 20.0f, -170.0f);
    b.p1 = DirectX::SimpleMath::Vector3(26.0f, 16.0f, -95.0f);
    b.p2 = DirectX::SimpleMath::Vector3(20.0f, 5.0f, -16.0f);
    b.p3 = DirectX::SimpleMath::Vector3(-28.0f, -12.0f, 40.0f);
    b.duration = 2.0f;
    m_playerPaths.push_back(b);

    //低い所からスッと横切る
    BezierPath c;
    c.p0 = DirectX::SimpleMath::Vector3(-166.0f, -6.0f, -150.0f);
    c.p1 = DirectX::SimpleMath::Vector3(-25.0f, -4.0f, -85.0f);
    c.p2 = DirectX::SimpleMath::Vector3( -4.0f, -2.0f, -12.0f);
    c.p3 = DirectX::SimpleMath::Vector3( 18.0f, -6.0f, 30.0f);
    c.duration = 1.6f;
    m_playerPaths.push_back(c);

    m_currentPathIndex = 0;
}

/// <summary>
/// 現在のパスを適応するための関数
/// </summary>
void TitleScene::ApplyCurrentPlayerPath()
{
    if (!m_titleMotion) { return; }
    if (m_playerPaths.empty()) { return; }

    const BezierPath& path = m_playerPaths[m_currentPathIndex];
    m_titleMotion->SetControlPoints(path.p0, path.p1, path.p2, path.p3);
    m_titleMotion->SetDuration(path.duration);
    m_titleMotion->ResetTime();
}

/// <summary>
/// ベジェ曲線用を次のパスに進める関数
/// </summary>
void TitleScene::AdvancePlayerPath()
{
    if (m_playerPaths.empty()) { return; }

    m_currentPathIndex++;

    if (m_currentPathIndex >= static_cast<int>(m_playerPaths.size()))
    {
        if (m_loopPaths)
        {
            m_currentPathIndex = 0;
        }
        else
        {
            m_currentPathIndex = static_cast<int>(m_playerPaths.size()) - 1;
        }
    }

    ApplyCurrentPlayerPath();
}

