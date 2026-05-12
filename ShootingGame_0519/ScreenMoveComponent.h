#pragma once
#include "Component.h"
#include "ForwardMoveComponent.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class ForwardMoveComponent;

class ScreenMoveComponent : public Component
{
public:
	void Initialize() override;
	void Update(float dt) override;
	void Uninit() override;

	//----------Set関数----------
	void SetRailProvider(ForwardMoveComponent* forwardComp) { m_forwardComp = forwardComp; };
	void SetMoveRange(float range) { m_range = range; };
	void SetFollowPower(float power) { m_followPower = power; };
	void SetDamping(float damping) {m_damping = damping;};

	//----------Get関数----------
	Vector2 GetCurrentOffset() const { return m_currentOffset; }
	Vector2 GetTargetOffset() const  { return m_targetOffset; }
	Vector2 GetVelocity() const { return m_velocity; }

private:
	//---------移動設定関連---------
	float m_range = 8.0f;		 //画面で動ける範囲
	float m_damping = 0.88f;	 //動く時の慣性
	float m_followPower= 40.0f;  //マウスへの追従の強さ

	//---------画面内移動移動設定関連---------
	Vector2 m_targetOffset;   //レティクル目標
	Vector2 m_currentOffset;  //現在位置
	Vector2 m_velocity;       //慣性用

	//参照関連
	ForwardMoveComponent* m_forwardComp;
};
