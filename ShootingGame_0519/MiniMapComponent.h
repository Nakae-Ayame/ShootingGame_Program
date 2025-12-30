#pragma once
#include "Component.h"
#include <string>
#include <SimpleMath.h>
#include <wrl/client.h>
#include <d3d11.h>

/// <summary>
/// ミニマップを実装するためのコンポ―ネント
/// 追跡対象のオブジェクト(プレイヤー、敵、建物など)を
/// ミニマップ上にアイコンで表示する
/// 
/// Playerの位置を中心に周辺のオブジェクトの位置を計算し表示する
/// m_coverageRadiusで正規化し、ミニマップ上での座標に変換する
/// </summary>
class MiniMapComponent : public Component
{
public:
	MiniMapComponent() {};
	~MiniMapComponent() override = default;

	void Initialize() override {};
	void Update(float dt) override {};
	void Draw(float alpha) override;

	//-----------Set関数-------------
	void SetScreenPosition(float x, float y) { m_screenPos = { x,y }; }
	void SetSize(float width, float height) { m_size = { width ,height }; }
	void SetCoverageRadius(float radius) { m_coverageRadius = radius; }
	void SetRotateWithPlayer(bool rotate) { m_rotateWithPlayer = rotate; }
	void SetPlayer(GameObject* player) { m_player = player; }
	void SetIconSize(float sizePx) { m_iconSizePx = sizePx; }

	void SetBackgroundSRV(ID3D11ShaderResourceView* srv) { m_backgroundSRV = srv; }
	void SetPlayerIconSRV(ID3D11ShaderResourceView* srv) { m_playerIconSRV = srv; }
	void SetEnemyIconSRV(ID3D11ShaderResourceView* srv) { m_enemyIconSRV = srv; }
	void SetBuildingIconSRV(ID3D11ShaderResourceView* srv) { m_buildingIconSRV = srv; }

	void SetEnemies(const std::vector<GameObject*>& enemies) { m_enemies = enemies; }
	void SetBuildings(const std::vector<GameObject*>& buildings) { m_buildings = buildings; }

	//-----------Get関数-------------
	DirectX::SimpleMath::Vector2 GetScreenPosition() const { return m_screenPos      ; }
	DirectX::SimpleMath::Vector2 GetSize() const { return m_size; }

private:
	DirectX::SimpleMath::Vector2 WorldToMiniMap(const DirectX::SimpleMath::Vector3& worldPos,
												const DirectX::SimpleMath::Vector3& playerPos,
												float playeryaw)const;

	//-----------描画関連変数-------------
	DirectX::SimpleMath::Vector2 m_screenPos{ 0.0f,0.0f };		//ミニマップのスクリーン座標
	DirectX::SimpleMath::Vector2 m_size{ 256.0f,256.0f };
	float m_iconSizePx = 10.0f;

	ID3D11ShaderResourceView* m_backgroundSRV   = nullptr;
	ID3D11ShaderResourceView* m_playerIconSRV   = nullptr;
	ID3D11ShaderResourceView* m_enemyIconSRV    = nullptr;
	ID3D11ShaderResourceView* m_buildingIconSRV = nullptr;

	//-----------ミニマップ変換変数-------------
	float m_coverageRadius = 200.0f; //ミニマップが写す範囲の半径
	bool m_rotateWithPlayer = false;  //プレイヤーの向きに合わせて回転するか

	//-------------追跡対象関連---------------
	GameObject* m_player = nullptr;
	std::vector<GameObject*> m_enemies;
	std::vector<GameObject*> m_buildings;
};