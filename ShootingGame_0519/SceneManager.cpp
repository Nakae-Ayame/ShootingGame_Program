#include <Windows.h> 
#include <iostream>
#include "SceneManager.h"
#include "TransitionManager.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "ResultScene.h"
#include "CollisionManager.h"
#include "Application.h" 
       
std::unordered_map<std::string, std::unique_ptr<IScene>> SceneManager::m_scenes;

std::string SceneManager::m_currentSceneName;

bool SceneManager::IsSceneChange = false;

void SceneManager::RegisterScene(const std::string& name, std::unique_ptr<IScene> scene)
{
    m_scenes[name] = std::move(scene); //m_sceneに引数のScene名とスーマートポインタを使って登録
}

void SceneManager::SetCurrentScene(const std::string& name)
{
    // 変更前のシーンがあれば一度 Uninit
    if (!m_currentSceneName.empty() && m_scenes.count(m_currentSceneName))
    {
        m_scenes[m_currentSceneName]->Uninit();
    }

    Input::Reset();

    // 新しいシーン名をセット＆Init
    m_currentSceneName = name;
    m_scenes[m_currentSceneName]->Init();

    // ウィンドウタイトルも変更（std::string → wchar_t* へ変換）
    int wlen = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
    std::wstring wname(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], wlen);

    HWND hWnd = Application::GetWindow();
    SetWindowTextW(hWnd, wname.c_str());
}

std::string SceneManager::GetCurrentSceneName()
{
    return m_currentSceneName;
}

void SceneManager::Init()
{
    //-------------------------------------------------------------
    //例）TitleSceneを登録
    //RegisterScene("TitleScene", std::make_unique<TitleScene>());
    //m_scenes["TitleScene"]->init();
    //-------------------------------------------------------------
 /*   RegisterScene("TitleScene", std::make_unique<TitleScene>());
    m_scenes["TitleScene"]->Init();*/
    RegisterScene("TitleScene", std::make_unique<TitleScene>());
    //m_scenes["TitleScene"]->Init();
    RegisterScene("GameScene", std::make_unique<GameScene>());
    //m_scenes["GameScene"]->Init();
    RegisterScene("ResultScene", std::make_unique<ResultScene>());
    //m_scenes["GameScene"]->Init();
    //初期シーンにTitleSceneを設定
    m_currentSceneName = "TitleScene";
    m_scenes[m_currentSceneName]->Init();

}

void SceneManager::Update(float deltatime)
{
    //入力アップデート
    Input::Update();

    //フレーム先頭で前フレームのコライダーをクリア
    CollisionManager::Clear();

    //トランジション更新（そのまま）
    TransitionManager::Update(deltatime);

    if (!TransitionManager::IsActive())
    {
        //SceneのUpdate（ここで Scene 内が RegisterCollider を呼ぶ）
        if (!m_currentSceneName.empty() && m_scenes.count(m_currentSceneName))
        {
            m_scenes[m_currentSceneName]->Update(deltatime);

            /*if (IsSceneChange)
            {
                return;
            }*/
        }

        // 4) 判定を実行（Scene がコライダーを登録し終えた後で）
        CollisionManager::CheckCollisions();

        // 5) 削除や追加の反映（安全なタイミング）
        if (!m_currentSceneName.empty() && m_scenes.count(m_currentSceneName))
        {
            m_scenes[m_currentSceneName]->FinishFrameCleanup();
        }
    }
    else
    {
        // 遷移中の扱い（従来通り）
    }
}



void SceneManager::Draw(float deltatime)
{
    // 1) 現在シーンを描画
    if (!m_currentSceneName.empty() && m_scenes.count(m_currentSceneName))
    {
        m_scenes[m_currentSceneName]->Draw(deltatime);
    }

    // 2) 遷移オーバーレイを上に重ねる（例：フェード）
    TransitionManager::Draw(deltatime);
}


void SceneManager::Uninit()
{
    // 登録されているすべてシーンの終了処理
    for (auto& s : m_scenes)
    {
        s.second->Uninit();
    }

    m_scenes.clear();
    m_currentSceneName.clear();
}