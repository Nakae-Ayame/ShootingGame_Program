#pragma once
#include <vector>
#include <d3d11.h>
#include "IScene.h"
#include "TitleBackGround.h"
#include "TitlePlayerMotionComponent.h"
#include "CameraObject.h"
#include "SkyDome.h"

//---------------------------------
//ISceneを継承したGameScene
//---------------------------------
class TitleScene : public IScene
{
public:
	explicit TitleScene() {};
	void Update(float deltatime) override;
	void Draw(float deltatime) override;
	//3Dワールド上の描画関数
	void DrawWorld(float deltatime) override;
	//UI上の描画関数
	void DrawUI(float deltatime) override;
	void Init() override;
	void Uninit() override;
	void AddObject(std::shared_ptr<GameObject> obj) override;
	void AddTextureObject(std::shared_ptr<GameObject> obj);
	void RemoveObject(std::shared_ptr<GameObject>) override;
	void RemoveObject(GameObject* obj);
	void FinishFrameCleanup() override;
	const std::vector<std::shared_ptr<GameObject>>& GetObjects() const override { return m_gameObjects; }
private:

	void SetSceneObject();
	std::vector<std::shared_ptr<GameObject>> m_gameObjects;
	std::vector<std::shared_ptr<GameObject>> m_textureObjects;
	std::vector<std::shared_ptr<GameObject>> m_deleteObjects;
	std::vector<std::shared_ptr<GameObject>> m_addObjects;

	std::shared_ptr<SkyDome> m_skyDome;
	std::shared_ptr<GameObject> m_player;
	std::shared_ptr<CameraObject> m_camera;

	std::shared_ptr<GameObject> m_titleLogo;	//タイトルロゴオブジェクト
	std::shared_ptr<GameObject> m_titleText;	//タイトルテキストオブジェクト

	std::shared_ptr<TitlePlayerMotionComponent> m_titleMotion;
	bool m_isLogoShown = false;				//2Dロゴが表示されたかどうか

	//--------------ロゴ点滅関連------------------
	float m_blinkTimer = 0.0f;
	float m_blinkInterval = 0.5f;
	bool m_blinkVisible = true;

	std::vector<BezierPath> m_playerPaths;
	int m_currentPathIndex = 0;
	bool m_loopPaths = true;

	void SetupPlayerPaths();
	void ApplyCurrentPlayerPath();
	void AdvancePlayerPath();
};



