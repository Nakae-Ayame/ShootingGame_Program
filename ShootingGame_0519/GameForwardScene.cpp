#include "GameForwardScene.h"
#include "ForwardMoveComponent.h"
#include "ForwardFollowCameraComponent.h"
#include "ScreenMoveComponent.h"
#include "RailShooterPlayer.h"
#include "AimRotateComponent.h"
#include "CameraObject.h"
#include "ModelComponent.h"
#include "FloorComponent.h"
#include "CsvGridLoader.h"
#include "renderer.h"

void GameForwardScene::Init()
{
    auto player = std::make_shared<RailShooterPlayer>();

    auto screenMove  = player->AddComponent<ScreenMoveComponent>();
    auto forwardMove = player->AddComponent<ForwardMoveComponent>();
    auto aimRotate   = player->AddComponent<AimRotateComponent>();

    screenMove->SetRailProvider(forwardMove.get());

    forwardMove->SetSpeed(30.0f);

    aimRotate->SetRailProvider(forwardMove.get());
    aimRotate->SetScreenMoveComponent(screenMove.get());

    aimRotate->SetBaseRotation(Vector3(0.0f, 0.0f, 0.0f));

    aimRotate->SetMaxYawAngle(2.0f);
    aimRotate->SetMaxPitchAngle(1.0f);
    aimRotate->SetMaxRollAngle(0.0f);
    aimRotate->SetTiltVelocityRange(14.0f);
    aimRotate->SetRotateLerpSpeed(3.0f);

    AddObject(player);

    auto tree = std::make_shared<GameObject>();

    //ÉÇÉfÉãê∂ê¨
    auto modelComp = std::make_shared<ModelComponent>();
    modelComp->LoadModel("Asset/Build/tree01.obj");
    modelComp->SetColor(Color(1, 0, 0, 1));
    tree->AddComponent(modelComp);

    tree->SetScale({ 1.0f, 1.0f, 1.0f });
    tree->SetPosition({ 0.0f, 100.0f, 300.0f });
    AddObject(tree);

    auto camera = std::make_shared<CameraObject>();
    m_cameraComp = camera->AddComponent<ForwardFollowCameraComponent>();

    m_cameraComp->SetTarget(player.get());
    m_cameraComp->SetRailProvider(forwardMove.get());

    AddObject(camera);

    for (auto& obj : m_gameObjects)
    {
        if (obj)
        {
            obj->Initialize();
        }
    }

    auto grid = CsvGridLoader::LoadGrid("Data/ForwardStage01.csv");

    float cellSize = 10.0f;

    int rowCount = grid.size();
    int colCount = grid[0].size();

    // íÜâõäÓèÄÇ…Ç∑ÇÈ
    int centerCol = colCount / 2;

    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            int value = grid[row][col];

            float x = (col - centerCol) * cellSize;
            float z = row * cellSize;
            float y = 0.0f;

            if (value == 1)
            {
                auto tree = std::make_shared<GameObject>();

                auto model = tree->AddComponent<ModelComponent>();
                model->LoadModel("Asset/Build/tree_0817053737_refine.obj");

                tree->SetPosition({ x, y, z });
                tree->SetScale({ 10.0, 10.0, 10.0 });

                AddObject(tree);
            }
            else if (value == 2)
            {
                /*auto enemy = std::make_shared<GameObject>();

                auto model = enemy->AddComponent<ModelComponent>();
                model->LoadModel("Asset/Model/Enemy/enemy.obj");

                enemy->SetPosition({ x, y, z });*/

                //AddObject(enemy);
            }
        }
    }
}

void GameForwardScene::Update(float deltatime)
{
    for (auto& obj : m_gameObjects)
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

    for (auto& obj : m_gameObjects)
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
    for (auto& obj : m_gameObjects)
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
    m_addObjects.push_back(obj);
}

void GameForwardScene::RemoveObject(std::shared_ptr<GameObject> obj)
{
    if (!obj) { return; }
    m_deleteObjects.push_back(obj);
}

void GameForwardScene::RemoveObject(GameObject* obj)
{
    if (!obj) { return; }

    for (const auto& sp : m_gameObjects)
    {
        if (sp.get() == obj)
        {
            sp->Initialize();
            m_deleteObjects.push_back(sp);
            return;
        }
    }
}

void GameForwardScene::FinishFrameCleanup()
{
    //----------------------------
    // AddîΩâf
    //----------------------------
    for (auto& obj : m_addObjects)
    {
        if (!obj)
        {
            continue;
        }

        obj->Initialize();
        m_gameObjects.push_back(obj);
    }
    m_addObjects.clear();

    //----------------------------
    // DeleteîΩâf
    //----------------------------
    if (!m_deleteObjects.empty())
    {
        for (auto& del : m_deleteObjects)
        {
            if (!del)
            {
                continue;
            }

            for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
            {
                if (*it == del)
                {
                    (*it)->Uninit();
                    m_gameObjects.erase(it);
                    break;
                }
            }
        }

        m_deleteObjects.clear();
    }
}

const std::vector<std::shared_ptr<GameObject>>& GameForwardScene::GetObjects() const
{
    return m_gameObjects;
}