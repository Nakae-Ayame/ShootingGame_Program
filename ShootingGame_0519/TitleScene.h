#pragma once
#include <vector>
#include <d3d11.h>
#include "IScene.h"
#include "TitleBackGround.h"

//---------------------------------
//ISceneÇåpè≥ÇµÇΩGameScene
//---------------------------------
class TitleScene : public IScene
{
public:
	explicit TitleScene() {};
	void Update(float deltatime) override;
	void Draw(float deltatime) override;
	void Init() override;
	void Uninit() override;
	void AddObject(std::shared_ptr<GameObject> obj) override;
	void RemoveObject(std::shared_ptr<GameObject>) override;
	void RemoveObject(GameObject* obj);
	void FinishFrameCleanup() override;
private:

	void SetSceneObject();
	std::vector<std::shared_ptr<GameObject>> m_GameObjects;
	std::vector<std::shared_ptr<GameObject>> m_DeleteObjects;
	std::vector<std::shared_ptr<GameObject>> m_AddObjects;
};



