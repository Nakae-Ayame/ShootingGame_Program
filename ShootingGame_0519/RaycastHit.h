#pragma once
#include <memory>
#include <SimpleMath.h>

class GameObject;

struct  RaycastHit
{
	DirectX::SimpleMath::Vector3 position;  //ヒット位置
	DirectX::SimpleMath::Vector3 normal;    //ヒット法線
	float distance = 0.0f;                  //ヒット距離
	std::shared_ptr<GameObject> hitObject;  //ヒットしたオブジェクト
};