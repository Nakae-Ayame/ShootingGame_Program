#pragma once
#include <vector>
#include <memory>
#include <SimpleMath.h>
#include <string>
#include "ModelResource.h"

//建物の初期設定
struct BuildingConfig
{
    std::string modelPath;                  //ファイルパス
    int count = 10;                         //建物の数
    float areaWidth = 10.0f;               //建物幅
    float areaDepth = 10.0f;               //建物奥行き
    float spacing = 10.0f;                  //建物の配置間隔
    bool randomizeRotation = true;          //
    float minScale = 75.0f, maxScale = 75.0f; //大きさの最小/最大
};

class GameScene;

class BuildingSpawner
{
public:
    BuildingSpawner(GameScene* scene);
    void Spawn(const BuildingConfig& cfg);

private:
    GameScene* m_scene;
};
