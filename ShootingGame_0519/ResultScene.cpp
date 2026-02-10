#include <iostream>
#include "ResultScene.h"
#include "Input.h"
#include "Player.h"
#include "TitleBackGround.h"
#include "Input.h"
#include "TransitionManager.h"

void ResultScene::Init()
{
    auto background01 = std::make_shared<TitleBackGround>(L"Asset/UI/ResultBackGround_Clear.jpg", 1280.0f);
    AddObject(background01);
    //AddObject(background02);

    for (auto& obj : m_AddObjects)
    {
        if (obj) obj->Initialize();
    }
}

void ResultScene::Update(float deltatime)
{
    //Input::Update();

    SetSceneObject();

    if (Input::IsKeyDown(VK_RETURN))
    {
        //旧式はこっち
        SceneManager::SetCurrentScene("TitleScene");
        //TransitionManager::Start("GameScene", 1.0f, TransitionType::FADE, nullptr);
    }


    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Update(deltatime);
    }
}

void ResultScene::Draw(float deltatime)
{
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Draw(deltatime);
    }
}


void ResultScene::DrawWorld(float deltatime)
{
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Draw(deltatime);
    }
}

void ResultScene::DrawUI(float deltatime)
{

}

void ResultScene::Uninit()
{

}


void ResultScene::AddObject(std::shared_ptr<GameObject> obj)
{
    if (!obj) return;
    // シーン参照を GameObject に教えておく（後述）
    obj->SetScene(this);
    m_AddObjects.push_back(obj);
}


void ResultScene::RemoveObject(std::shared_ptr<GameObject> obj)
{
    m_DeleteObjects.push_back(obj);
}

void ResultScene::RemoveObject(GameObject* obj)
{
    //if (!obj) return;
    //// 重複追加を防ぎたい場合チェックしてから push_back してもよい
    //m_DeleteObjects.push_back(obj);
}

void ResultScene::FinishFrameCleanup()
{
    for (auto& p : m_DeleteObjects) // p は shared_ptr<GameObject>
    {
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(), [&](const std::shared_ptr<GameObject>& sp)
            {return sp == p; });
        if (it != m_GameObjects.end()) m_GameObjects.erase(it);
    }
    m_DeleteObjects.clear();
}

void ResultScene::SetSceneObject()
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
