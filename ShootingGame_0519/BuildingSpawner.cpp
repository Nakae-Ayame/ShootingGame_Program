#include "BuildingSpawner.h"
#include "GameObject.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "BoxComponent.h"
#include "OBBColliderComponent.h"
#include "ModelCache.h"
#include "Building.h"
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
    for (int i = 0; i < cfg.count; ++i)
    {
        float x = RandFloat(-cfg.areaWidth * 0.5f, cfg.areaWidth * 0.5f);
        float z = RandFloat(-cfg.areaDepth * 0.5f, cfg.areaDepth * 0.5f);

        auto obj = std::make_shared<Building>();
        obj->SetScene(m_scene);
        obj->SetPosition({ x, -12, z }); // 高さは統一
        obj->SetScale({ 10, 10, 10 });

        if (cfg.randomizeRotation)
        {
            obj->SetRotation({ 0.0f, RandFloat(0.0f, 6.2831853f), 0.0f });
        }
        else
        {
            obj->SetRotation({ 0.0f, 0.0f, 0.0f });
        }

        // AABB コライダーを作って必ずオブジェクトに追加すること（重要）
        auto col = std::make_shared<OBBColliderComponent>();
        col->SetSize({ 3.0f, 3.0f, 3.0f }); // 必要に応じてモデルサイズに合わせる
        obj->AddComponent(col);

        auto mc = std::make_shared<ModelComponent>();
        mc->LoadModel(cfg.modelPath);
        obj->AddComponent(mc);

        obj->Initialize();
        m_scene->AddObject(obj);
    }

}