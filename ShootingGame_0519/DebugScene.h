#pragma once
#include <vector>
#include <d3d11.h>
#include "IScene.h"
#include "FreeCamera.h"
#include "Player.h"
#include "CameraObject.h"
#include "Enemy.h"
#include "TextureComponent.h"
#include "DebugRenderer.h"
#include "Reticle.h"
#include "SkyDome.h"
#include "Bullet.h"
#include "TitleBackGround.h"
#include "DebugUI.h"
#include "HPBar.h"
#include "EnemySpawner.h"
#include "BuildingSpawner.h"
#include "MoveComponent.h"
#include "MiniMapComponent.h"

//---------------------------------
//ISceneを継承したDebugScene
//---------------------------------
enum class DebugState
{
	Countdown,
	Playing,
};

class DebugScene : public IScene
{
public:
	explicit DebugScene() {};

	void Update(float deltatime) override;
	void Draw(float deltatime) override;
	void DrawWorld(float deltatime) override;
	void DrawUI(float deltatime) override;
	void Init() override;
	void Uninit() override;

	//------------------IMGUI用関数------------------
	void DebugConfigWindow();

	//-------------オブジェクト関連の関数-------------
	void AddObject(std::shared_ptr<GameObject> obj) override;
	void AddTextureObject(std::shared_ptr<GameObject> obj);
	void RemoveObject(std::shared_ptr<GameObject> obj) override {};
	void RemoveObject(GameObject* obj) override;
	void FinishFrameCleanup() override;

	const std::vector<std::shared_ptr<GameObject>>& GetObjects() const override { return m_GameObjects; }

	bool Raycast(
		const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& dir,
		float maxDistance,
		RaycastHit& outHit,
		std::function<bool(GameObject*)> predicate,
		GameObject* ignore = nullptr) override;

	//---------フレーム終了時に削除・追加予定のオブジェクト配列---------
	std::vector<std::shared_ptr<GameObject>> m_DeleteObjects;
	std::vector<std::shared_ptr<GameObject>> m_AddObjects;

private:
	DebugState m_gameState = DebugState::Countdown;

	float m_countdownRemaining = 4.0f;		//開始カウントダウン用

	float m_aimMarginX = 80.0f;   // 左右マージン（好きな値に変えてOK）
	float m_aimMarginY = 45.0f;   // 上下マージン

	FollowCameraComponent* m_cameraComp = nullptr;

	std::unique_ptr<EnemySpawner> m_enemySpawner;

	std::unique_ptr<BuildingSpawner> m_buildingSpawner;

	std::unique_ptr<DebugRenderer> m_debugRenderer;

	std::shared_ptr<Player> m_player;
	std::shared_ptr<CameraObject> m_FollowCamera;
	std::shared_ptr<SkyDome> m_SkyDome;

	std::shared_ptr<GameObject> m_CountDown01;
	std::shared_ptr<GameObject> m_CountDown02;
	std::shared_ptr<GameObject> m_CountDown03;
	std::shared_ptr<GameObject> m_CountDownGo;
	std::shared_ptr<GameObject> m_CountDownNow;

	//GameScene内の3Dオブジェクトの配列
	std::vector<std::shared_ptr<GameObject>> m_GameObjects;

	//GameScene内の2Dオブジェクトの配列
	std::vector<std::shared_ptr<GameObject>> m_TextureObjects;

	// --- レティクル関係 ---
	std::shared_ptr<GameObject> m_reticleObj;           // レティクル用 GameObject（描画のみでコンポーネント持つ）
	std::shared_ptr<HPBar> m_HPObj;           // レティクル用 GameObject（描画のみでコンポーネント持つ）
	std::shared_ptr<TextureComponent> m_reticleTex;     // レティクルのテクスチャコンポーネント

	bool m_isDragging = false;      // ドラッグ中フラグ
	POINT m_lastDragPos{ 0,0 };     // 最終置いたスクリーン座標

	float m_reticleW = 64.0f;       // レティクル幅（px）
	float m_reticleH = 64.0f;       // レティクル高さ（px）

	//SceneのUpdate時に追加予定であったオブジェクトの配列の追加などを行う関数
	void SetSceneObject();

	std::shared_ptr<Reticle> m_reticle;

	bool isCollisionDebugMode = false;

	float setSpeed = 10.0f;

	float setAimDistance = 2000.0f;

	float motionX = 0.5f;
	float motionY = 0.5f;

	Vector3 setRot = { 0,0,0 };

	int enemyCount = 0;

	//std::shared_ptr<PlayAreaComponent> m_playArea;

	std::shared_ptr<MoveComponent> m_playerMove;

	//--------------ミニマップ関連------------------
	std::shared_ptr<GameObject> m_miniMapUi;
	MiniMapComponent* m_miniMap = nullptr;

	ID3D11ShaderResourceView* m_miniMapBgSRV = nullptr;
	ID3D11ShaderResourceView* m_miniMapPlayerSRV = nullptr;
	ID3D11ShaderResourceView* m_miniMapEnemySRV = nullptr;
	ID3D11ShaderResourceView* m_miniMapBuildingSRV = nullptr;

	//------------設定用ファイル関連------------------
	std::string m_iniPath = "Data/GameSettings.ini";

	//--------------Player設定関連------------------
	float m_playerMoveSpeed       = 35.0f;
	float m_playerBoostMultiplier = 2.0f;
	float m_playerBulletSpeed     = 300.0f;
	float m_playerHp              = 20.0f;

	//-------------Camera設定関連-----------------
	float m_cameraDistance  = 20.0f;
	float m_cameraHeight    = 6.0f;
	float m_cameraFovDeg    = 50.0f;
	float m_cameraBoostFovDeg = 65.0f;
	float m_cameraSensitivity = 1.0f;

	//--------------Blur設定関連------------------
	float m_blurStretch    = 0.5;
	float m_blurStartPoint = 0.2;
	float m_blurEndPoint   = 1.0;
	float m_blurCenterX = 0.5;
	float m_blurCenterY = 0.5;

	PostProcessSettings pp;

	//----------------初期化分業用関数--------------------
	void InitializeDebug();
	void InitializePlayArea();
	void InitializePhase();
	void InitializeCamera();
	void InitializePlayer();
	void InitializeEnemy();
	void InitializeStageObject();
	void InitializeUI();

	//--------------Ini設定関連------------------
	bool LoadPlayerConfigFromIni();
	void SavePlayerConfigToIni();

	std::string m_imguiMessageLog;

	//-------------ゲームセッティング関連--------------
	void DebugGameDateSet();

};
