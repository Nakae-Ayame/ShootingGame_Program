#pragma once
#include <vector>
#include <d3d11.h>
#include "IScene.h"

class GameObject;

class GameForwardScene : public IScene
{
public:
	explicit GameForwardScene() {};
	
	void Update(float deltatime) override;
	void Draw(float deltatime) override;
	void DrawWorld(float deltatime) override;
	void DrawUI(float deltatime) override;
	void Init() override;
	void Uninit() override;

private:
	//-------------Imgui関連関数-------------

	//------------Object用配列---------------
	std::vector<std::shared_ptr<GameObject>> m_GameObjects;    //3Dオブジェクト用配列
	std::vector<std::shared_ptr<GameObject>> m_TextureObjects; //2Dオブジェクト用配列

	std::vector<std::shared_ptr<GameObject>> m_DeleteObjects;  //削除予定オブジェクト用配列
	std::vector<std::shared_ptr<GameObject>> m_AddObjects;     //追加予定オブジェクト用配列
};
