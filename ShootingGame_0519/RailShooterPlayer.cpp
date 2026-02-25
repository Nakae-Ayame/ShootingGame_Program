#include "RailShooterPlayer.h"
#include <iostream>

void RailShooterPlayer::Initialize()
{
	//‰Šú‰»ˆ—
	GameObject::Initialize();
}

void RailShooterPlayer::Update(float dt)
{
	//XVˆ—
	GameObject::Update(dt);
}

void RailShooterPlayer::OnCollision(GameObject* other)
{
	//Õ“Ëˆ—
	std::cout << "RailShooterPlayer collided with " << typeid(*other).name() << std::endl;
}