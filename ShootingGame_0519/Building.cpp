#include "Building.h"

void Building::Initialize()
{
    //基底クラスの初期化（Component群）
    GameObject::Initialize();

    //モデルコンポーネントの生成
    auto modelComp = std::make_shared<ModelComponent>();

    //モデルの読み込み（失敗時に備えログなども可）
    modelComp->LoadModel("Asset/Model/Building1.fbx");

    //---------------GameObjectに追加---------------
    AddComponent(modelComp);
    //----------------------------------------------
}

void Building::Update(float dt)
{
    //コンポーネント等のUpdate
    GameObject::Update(dt);

}