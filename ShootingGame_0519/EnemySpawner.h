#pragma once
#include <vector>
#include <memory>
#include <SimpleMath.h>
#include "GameObject.h"

//PatrolEnemyの初期設定
struct PatrolConfig
{
	int spawnCount = 4;
	float speed    = 25.0f;
	float arrival = 0.5f;
	bool pingPong = true;
	std::vector<DirectX::SimpleMath::Vector3> waypoints;
};

//CirclePatrolEnemyの初期設定
struct CircleConfig
{
	int spawnCount = 3;
	DirectX::SimpleMath::Vector3 center = { 0,0,0 };
	float radius = 25.0f;
	float angularSpeed = DirectX::XM_PI / 2.0f;
	bool clockwise = true;
};

struct TurretConfig
{
	int spawnCount = 3;
	float coolTime = 1.0f;
	float bulletSpeed = 15.0;
	DirectX::SimpleMath::Vector3 pos = { 0,0,0 };
	std::weak_ptr<GameObject> target;
};

class GameScene;

class EnemySpawner
{
public:
	EnemySpawner(GameScene* scene);

	//設定オブジェクト
	PatrolConfig patrolCfg;
	CircleConfig circleCfg;
	TurretConfig turretCfg;

	//現在設定に合わせて生成又は破棄を行う
	void EnsurePatrolCount();
	void EnsureCircleCount();
	void EnsureTurretCount();

	//既存の敵に設定をすぐ反映する
	void ApplyPatrolSettingsToAll();
	void ApplyCircleSettingsToAll();
	void ApplyTurretSettingsToAll();

	// 全部消す
	void DestroyAll();

	void SetWaypoints(std::vector<DirectX::SimpleMath::Vector3> waypoint)
	{
		patrolWaypointSets.push_back(waypoint);
	}

	void SetRadius(float radius)
	{
		circlePatrolRadiusSets.push_back(radius);
	}

	void SetCircleCenter(DirectX::SimpleMath::Vector3 center)
	{
		circlePatrolCenterSets.push_back(center);
	}

	void SetTurretPos(DirectX::SimpleMath::Vector3 Pos)
	{
		TurretPosSets.push_back(Pos);
	}

private:
	GameScene* m_scene;

	//PatrolEnemyの移動中間地点の配列
	std::vector<std::vector<DirectX::SimpleMath::Vector3>> patrolWaypointSets;
	
	//CirclePatrolEnemyの半径の配列
	std::vector<float> circlePatrolRadiusSets;

	//CirclePatrolEnemy中心座標の配列
	std::vector<DirectX::SimpleMath::Vector3> circlePatrolCenterSets;

	//TurretPatrolEnemy中心座標の配列
	std::vector<DirectX::SimpleMath::Vector3> TurretPosSets;

	//生成した敵を管理（弱参照で持っておく）
	std::vector<std::weak_ptr<GameObject>> m_spawnedPatrols;
	std::vector<std::weak_ptr<GameObject>> m_spawnedCircles;
	std::vector<std::weak_ptr<GameObject>> m_spawnedTurrets;

	//EnemyFactory関数
	std::shared_ptr<GameObject> SpawnPatrolEnemy(const PatrolConfig& cfg, const DirectX::SimpleMath::Vector3& pos);
	std::shared_ptr<GameObject> SpawnCircleEnemy(const CircleConfig& cfg, const DirectX::SimpleMath::Vector3& pos);
	std::shared_ptr<GameObject> SpawnTurretEnemy(const TurretConfig& cfg, const DirectX::SimpleMath::Vector3& pos);

};