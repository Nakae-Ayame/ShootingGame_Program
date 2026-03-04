#include "GameForwardScene.h"
#include "ForwardMoveComponent.h"
#include "ForwardFollowCameraComponent.h"
#include "ScreenMoveComponent.h"
#include "RailShooterPlayer.h"
#include "CameraObject.h"
#include "renderer.h"

void GameForwardScene::Init()
{
    auto player = std::make_shared<RailShooterPlayer>();
    auto ScreenMove  = player->AddComponent<ScreenMoveComponent>();
    auto forwardMove = player->AddComponent<ForwardMoveComponent>();
   
    ScreenMove->SetRailProvider(forwardMove.get());

    forwardMove->SetSpeed(70.0f);

    AddObject(player);

    auto camera = std::make_shared<CameraObject>();
    m_cameraComp = camera->AddComponent<ForwardFollowCameraComponent>();

    m_cameraComp->SetTarget(player.get());
    m_cameraComp->SetRailProvider(forwardMove.get());

    AddObject(camera);

    for (auto& obj : m_GameObjects)
    {
        if (obj)
        {
            obj->Initialize();
        }
    }
}

void GameForwardScene::Update(float deltatime)
{
    for (auto& obj : m_GameObjects)
    {
        if (obj)
        {
            obj->Update(deltatime);
        }
    }
}

void GameForwardScene::Draw(float deltatime)
{

}

void GameForwardScene::DrawWorld(float deltatime)
{
    if (m_cameraComp)
    {
        Renderer::SetViewMatrix(m_cameraComp->GetView());
        Renderer::SetProjectionMatrix(m_cameraComp->GetProj());
    }

    for (auto& obj : m_GameObjects)
    {
        if (obj)
        {
            obj->Draw(deltatime);
        }
    }
}

void GameForwardScene::DrawUI(float deltatime)
{

}

void GameForwardScene::Uninit()
{
    for (auto& obj : m_GameObjects)
    {
        if (obj)
        {
            obj->Uninit();
        }
    }
}


void GameForwardScene::AddObject(std::shared_ptr<GameObject> obj)
{
    if (!obj) { return; }
    m_AddObjects.push_back(obj);
}

void GameForwardScene::RemoveObject(std::shared_ptr<GameObject> obj)
{
    if (!obj) { return; }
    m_DeleteObjects.push_back(obj);
}

void GameForwardScene::RemoveObject(GameObject* obj)
{
    if (!obj) { return; }

    for (const auto& sp : m_GameObjects)
    {
        if (sp.get() == obj)
        {
            sp->Initialize();
            m_DeleteObjects.push_back(sp);
            return;
        }
    }
}

void GameForwardScene::FinishFrameCleanup()
{
    //----------------------------
    // Add”½‰f
    //----------------------------
    for (auto& obj : m_AddObjects)
    {
        if (!obj)
        {
            continue;
        }

        obj->Initialize();
        m_GameObjects.push_back(obj);
    }
    m_AddObjects.clear();

    //----------------------------
    // Delete”½‰f
    //----------------------------
    if (!m_DeleteObjects.empty())
    {
        for (auto& del : m_DeleteObjects)
        {
            if (!del)
            {
                continue;
            }

            for (auto it = m_GameObjects.begin(); it != m_GameObjects.end(); ++it)
            {
                if (*it == del)
                {
                    (*it)->Uninit();
                    m_GameObjects.erase(it);
                    break;
                }
            }
        }

        m_DeleteObjects.clear();
    }
}

const std::vector<std::shared_ptr<GameObject>>& GameForwardScene::GetObjects() const
{
    return m_GameObjects;
}