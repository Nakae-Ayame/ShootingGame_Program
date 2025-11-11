#include "PatrolComponent.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

using namespace DirectX::SimpleMath;

void PatrolComponent::Initialize()
{
	//向かう地点が最低一つは設定されており、かつ今設定されているインデックスが
	//全ウェイポイント数以上の場合
	if (!m_Waypoints.empty() && m_CurrentIndex >= m_Waypoints.size())
	{
		m_CurrentIndex = 0;
	}

	//std::cout << "[PatrolComponent] Initialize: waypoints=" << m_Waypoints.size()
		//<< " startIndex=" << m_CurrentIndex << " speed=" << m_speed << "\n";
}

void PatrolComponent::Update(float dt)
{
	//親オブジェクトがないなら
	if (!GetOwner()){return;}
	//向かう地点が何もない場合
	if (m_Waypoints.empty()) { return; }
	//デルタタイムが0より小さい場合
	if (dt <= 0.0f) { return; }

	
	static float s_logAccum = 0.0f;
	s_logAccum += dt;
	bool doPeriodicLog = false;
	if (s_logAccum >= 1.0f) { doPeriodicLog = true; s_logAccum = 0.0f; }


	//コンポーネントのついているオブジェクトの位置を取得
	Vector3 pos = GetOwner()->GetPosition();

	//ターゲットをいま向かっている地点に設定
	Vector3 target = m_Waypoints[m_CurrentIndex];

	//ベクトル作成
	Vector3 dir = target - pos;
	//二乗計算
	float dist2 = dir.LengthSquared();
	//
	float thresh2 = m_arrivalThreshold * m_arrivalThreshold;

	if (dist2 <= thresh2)
	{
		/*std::cout << "[PatrolComponent] Reached waypoint idx=" << m_CurrentIndex
			<< " pos=(" << pos.x << "," << pos.y << "," << pos.z << ")\n";*/


		//コールバック呼び出し
		if (m_onReached) { m_onReached(m_CurrentIndex); }

		// 次のインデックス計算（既存のロジック）
		size_t prevIndex = m_CurrentIndex;

		if (m_PingPong)
		{
			//次インデックス = current + dir
			int next = static_cast<int>(m_CurrentIndex) + m_Dir;

			//nextが 0 より小さい 又は next がウェイポイント数以上なら
			if (next < 0 || next >= static_cast<int>(m_Waypoints.size()))
			{
				//方向転換する
				m_Dir =- m_Dir;

				next = static_cast<int>(m_CurrentIndex) + m_Dir;

				//もし単一点で反転後も範囲外なら clamp
				if (next < 0)
				{
					next = 0;
				}

				//next がウェイポイント数以上なら
				if (next >= static_cast<int>(m_Waypoints.size()))
				{
					next = static_cast<int>(m_Waypoints.size() - 1);
				}
			}
			m_CurrentIndex = static_cast<size_t>(next);
		}
		else
		{
			//ループ or 停止
			//今の地点 + 1 がウェイポイント数よりも小さかったら
			if (m_CurrentIndex + 1 < m_Waypoints.size())
			{
				//今のインデックス + 1
				m_CurrentIndex++;
			}
			else
			{
				if (m_loop)
				{
					m_CurrentIndex = 0;
				}
				else
				{
					// 到着したまま停止（有効フラグを折る）
					//m_Enabled = false;
				}
			}

			if (prevIndex != m_CurrentIndex)
			{
				//std::cout << "[PatrolComponent] Now targeting waypoint idx=" << m_CurrentIndex << "\n";
			}
		}
		return; // 到着時はまず向きを更新せず次フレームから移動
	}

	//移動処理
	//ベクトルがある程度の長さがあるなら
	if (dir.LengthSquared() > 1e-6f)
	{
		//ベクトルを正規化
		dir.Normalize();

		//ポジションを追加
		pos += dir * m_speed * dt;

		//Enemyの位置を更新
		GetOwner()->SetPosition(pos);

		//向き(yaw)の更新(X-Z平面の yaw を想定)
		if (m_FaceMovement)
		{
			Vector3 rot = GetOwner()->GetRotation();
			float yaw = std::atan2(dir.x, dir.z); // PatrolComponent と互換性を保つ atan2(x,z)
			rot.y = yaw;
			GetOwner()->SetRotation(rot);
		}
	}

	// 1秒に1回の状態ログ（スパム回避）
	if (doPeriodicLog)
	{
		/*std::cout << "[PatrolComponent] Pos=(" << pos.x << "," << pos.y << "," << pos.z
			<< ") TargetIdx=" << m_CurrentIndex
			<< " TargetPos=(" << target.x << "," << target.y << "," << target.z << ")\n";*/
	}
}

void PatrolComponent::SetWaypoints(const std::vector<Vector3>& pts)
{
	m_Waypoints = pts;
	if (m_CurrentIndex >= m_Waypoints.size())
	{
		m_CurrentIndex = 0;
	}
}

void PatrolComponent::SetWaypoints(std::vector<Vector3>&& pts)
{
	m_Waypoints = std::move(pts);
	if (m_CurrentIndex >= m_Waypoints.size())
	{
		m_CurrentIndex = 0;
	}
}

void PatrolComponent::Reset()
{
	m_CurrentIndex = 0;
	m_Dir = 1;
	//m_Enabled = true;
}

