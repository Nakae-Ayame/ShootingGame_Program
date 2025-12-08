#include <iostream>
#include "DebugScene.h"
#include "CameraObject.h"
#include "Input.h"
#include "DebugPlayer.h"

void DebugScene::Init()
{
    auto player = std::make_shared<DebugPlayer>();
    AddObject(player);

    auto camera = std::make_shared<CameraObject>();
    camera->Initialize();

    if (camera)
    {
        auto cameraComp = camera->GetComponent<FollowCameraComponent>();
		cameraComp->SetTarget(player.get());
    }

    AddObject(camera);

    for (auto& obj : m_AddObjects)
    {
        if (obj) obj->Initialize();
    }
}

void DebugScene::Update(float deltatime)
{
    SetSceneObject();

    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Update(deltatime);
    }
}

void DebugScene::Draw(float deltatime)
{
    for (auto& obj : m_GameObjects)
    {
        if (obj) obj->Draw(deltatime);
    }
}

void DebugScene::Uninit()
{

}


void DebugScene::AddObject(std::shared_ptr<GameObject> obj)
{
    if (!obj) return;
    // シーン参照を GameObject に教えておく（後述）
    obj->SetScene(this);
    m_AddObjects.push_back(obj);
}


void DebugScene::RemoveObject(std::shared_ptr<GameObject> obj)
{
    m_DeleteObjects.push_back(obj);
}

void DebugScene::RemoveObject(GameObject* obj)
{
    //if (!obj) return;
    //// 重複追加を防ぎたい場合チェックしてから push_back してもよい
    //m_DeleteObjects.push_back(obj);
}

void DebugScene::FinishFrameCleanup()
{
    for (auto& p : m_DeleteObjects) // p は shared_ptr<GameObject>
    {
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(), [&](const std::shared_ptr<GameObject>& sp)
            {return sp == p; });
        if (it != m_GameObjects.end()) m_GameObjects.erase(it);
    }
    m_DeleteObjects.clear();
}

void DebugScene::SetSceneObject()
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
