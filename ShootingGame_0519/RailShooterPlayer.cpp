#include <iostream>
#include "RailShooterPlayer.h"
#include "ModelComponent.h"

void RailShooterPlayer::Initialize()
{
	//モデルコンポーネントの生成
	auto modelComp = std::make_shared<ModelComponent>();
	//モデルの読み込み
	modelComp->LoadModel("Asset/Model/Player/Fighterjet.obj");
	modelComp->SetColor(Color(1, 0, 0, 1));

	AddComponent(modelComp);

	//初期化処理
	GameObject::Initialize();
}

void RailShooterPlayer::Update(float dt)
{
	//更新処理
	GameObject::Update(dt);

	std::cout << "Player Pos:" << GetPosition().x << ", " << GetPosition().y << ", " << GetPosition().z << std::endl;
}

void RailShooterPlayer::OnCollision(GameObject* other)
{

}