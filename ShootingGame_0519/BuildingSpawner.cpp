#include "BuildingSpawner.h"
#include "GameObject.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "AABBColliderComponent.h"
#include "ModelCache.h"
#include <random>

BuildingSpawner::BuildingSpawner(GameScene* scene) : m_scene(scene)
{
  
}

static float RandFloat(float a, float b)
{
    static std::mt19937_64 rng(123456);
    std::uniform_real_distribution<float> d(a, b);
    return d(rng);
}

void BuildingSpawner::Spawn(const BuildingConfig& cfg)
{
    if (!m_scene) { return; }
    /*auto res = ModelCache::Instance().GetOrLoad(cfg.modelPath);
    if (!res)
    {
        OutputDebugStringA("BuildingSpawner::Spawn - model load failed\n");
        return;
    }*/

    //簡単にグリッドを配置しておく
    int perRow = static_cast<int>(std::ceil(std::sqrt((double)cfg.count)));
    float startX = -cfg.areaWidth * 0.5f;
    float startZ = -cfg.areaDepth * 0.5f;
    float stepX;
    if (cfg.count > 1)
    {
        stepX = cfg.areaWidth / perRow;
    }
    else
    {
        stepX = cfg.spacing;
    }
    float stepZ = stepX;

    int placed = 0;
    for (int r = 0; r < perRow && placed < cfg.count; ++r)
    {
        for (int c = 0; c < perRow && placed < cfg.count; ++c)
        {
            //奥行と位置を設定(ある程度ランダム)
            float x = startX + c * stepX + RandFloat(-cfg.spacing * 0.3f, cfg.spacing * 0.3f);
            float z = startZ + r * stepZ + RandFloat(-cfg.spacing * 0.3f, cfg.spacing * 0.3f);

            //建物生成
            auto obj = std::make_shared<GameObject>();
            obj->SetScene(m_scene);
            obj->SetPosition({ x, 0.0f, z });
            float scale = RandFloat(cfg.minScale, cfg.maxScale);
            obj->SetScale({ 20, 20, 20 });

            if (cfg.randomizeRotation)
            {
                obj->SetRotation({ 0.0f, RandFloat(0.0f, 6.2831853f), 0.0f });
            }

            //ModelComponentを作るがリソースは共有(今はまだ実装していない)
            auto mc = std::make_shared<ModelComponent>();
            //mc->SetResource(res);
            mc->LoadModel(cfg.modelPath);
            obj->AddComponent(mc);

            //建物なのでAABB
            auto aabb = std::make_shared<AABBColliderComponent>();

            //ざっくりサイズを設定する
            aabb->SetSize({ 5.0f * 20, 10.0f * 20, 5.0f * 20 });
            obj->AddComponent(aabb);

            obj->Initialize();

            m_scene->AddObject(obj);
            ++placed;
        }
    }
}