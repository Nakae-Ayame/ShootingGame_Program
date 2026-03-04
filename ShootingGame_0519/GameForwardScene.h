#pragma once
#include <vector>
#include <d3d11.h>
#include "IScene.h"

class GameObject;
class ForwardFollowCameraComponent;

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

	void AddObject(std::shared_ptr<GameObject> obj) override;
	void RemoveObject(std::shared_ptr<GameObject> obj) override ;
	void RemoveObject(GameObject* obj) override;
	const std::vector<std::shared_ptr<GameObject>>& GetObjects() const override;
	void FinishFrameCleanup();

private:
	//-------------Imgui関連関数-------------

	//------------Object用配列---------------
	std::vector<std::shared_ptr<GameObject>> m_GameObjects;    //3Dオブジェクト用配列
	std::vector<std::shared_ptr<GameObject>> m_TextureObjects; //2Dオブジェクト用配列

	std::vector<std::shared_ptr<GameObject>> m_DeleteObjects;  //削除予定オブジェクト用配列
	std::vector<std::shared_ptr<GameObject>> m_AddObjects;     //追加予定オブジェクト用配列

	std::shared_ptr<ForwardFollowCameraComponent> m_cameraComp; // カメラコンポーネントへのキャッシュ生ポインタ
};
