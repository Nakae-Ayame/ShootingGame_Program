#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <SimpleMath.h>
#include "RaycastHit.h"

/// <summary>
/// Sceneを作る際などに使うインターフェースとしてのクラス
/// </summary>
class IScene
{
public:

	IScene() = default;
	virtual ~IScene() = default; 

	//-------------一連の流れ-------------
	virtual void Update(float delta) = 0; 
	virtual void Draw(float delta) = 0;	 

	virtual void DrawWorld(float dt) {};
	virtual void DrawUI(float dt) {};

	virtual void Init() = 0;				 
	virtual void Uninit() = 0;	

	//---------------他からオブジェクトの追加を行う関数--------------
	virtual void AddObject(std::shared_ptr<class GameObject> obj) = 0;

	virtual void RemoveObject(std::shared_ptr<GameObject> obj) = 0;
	virtual void RemoveObject(GameObject* obj) = 0;
	
	virtual void FinishFrameCleanup() = 0;
	//-----------------Raycast------------------

	virtual bool Raycast(
		const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& dir,
		float maxDistance,
		RaycastHit& outHit,
		std::function<bool(GameObject*)> predicate,
		GameObject* ignore = nullptr) { return false; }

	//---------------シーン内にあるオブジェクトを持ってくる関数------------------
	virtual const std::vector<std::shared_ptr<GameObject>>& GetObjects() const = 0;
};
