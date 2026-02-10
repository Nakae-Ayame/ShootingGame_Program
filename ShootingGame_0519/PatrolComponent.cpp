#include "PatrolComponent.h"
#include "GameObject.h"
#include <algorithm>
#include <cmath>

using namespace DirectX::SimpleMath;

void PatrolComponent::Initialize()
{
	//向かう地点が最低一つは設定されており、かつ今設定されているインデックスが
	//全ウェイポイント数以上の場合
	if (!m_waypoints.empty() && m_currentIndex >= m_waypoints.size())
	{
		m_currentIndex = 0;
	}
}

void PatrolComponent::Update(float dt)
{
    if (!GetOwner()){ return; }

    if (m_waypoints.size() < 2){ return; }
    
    if (dt <= 0.0f){ return; }
    
    if (!m_useSpline){ return; }

    GameObject* owner = GetOwner();

    //--- 制御点取得 ---
    auto calcPoints = [&](Vector3& p0, Vector3& p1, Vector3& p2, Vector3& p3)
        {
            int i1 = static_cast<int>(m_currentIndex);
            int i2 = i1 + 1;
            int i0 = i1 - 1;
            int i3 = i2 + 1;

            p0 = GetPointClamped(i0);
            p1 = GetPointClamped(i1);
            p2 = GetPointClamped(i2);
            p3 = GetPointClamped(i3);
        };

    Vector3 p0, p1, p2, p3;
    calcPoints(p0, p1, p2, p3);

    //--- spine 上の進行 ---
    Vector3 tangent = EvalCatmullRomTangent(p0, p1, p2, p3, m_segmentT);
    float tanLen = tangent.Length();
    if (tanLen < 1e-6f)
    {
        AdvanceSegment();
        return;
    }

    m_segmentT += (m_speed * dt) / tanLen;

    if (m_segmentT >= 1.0f)
    {
        m_segmentT -= 1.0f;
        AdvanceSegment();
        calcPoints(p0, p1, p2, p3);
    }

    //==============================
    // Reynolds: 未来位置の予測
    //==============================
    Vector3 pos = owner->GetPosition();
    Vector3 predictedPos = pos + (m_currentDir * m_speed * m_lookAheadTime);

    // spine 上の対応点（簡易：現在区間）
    Vector3 onPathPos = EvalCatmullRom(p0, p1, p2, p3, m_segmentT);

    float distFromPath = (predictedPos - onPathPos).Length();

    Vector3 desiredDir = m_currentDir;

    //==============================
    // Reynolds: 半径外なら Seek
    //==============================
    if (distFromPath > m_pathRadius)
    {
        desiredDir = onPathPos - pos;
        if (desiredDir.LengthSquared() > 1e-6f)
        {
            desiredDir.Normalize();
        }
    }

    //==============================
    // ステアリング（旋回制限）
    //==============================
    Vector3 curDir = m_currentDir;
    if (curDir.LengthSquared() < 1e-6f)
    {
        curDir = desiredDir;
    }
    else
    {
        curDir.Normalize();
    }

    float alpha = m_turnRate * dt;
    if (alpha > 1.0f)
    {
        alpha = 1.0f;
    }

    curDir = Vector3::Lerp(curDir, desiredDir, alpha);
    if (curDir.LengthSquared() > 1e-6f)
    {
        curDir.Normalize();
    }

    m_currentDir = curDir;

    //==============================
    // 位置更新（path に張り付かない）
    //==============================
    Vector3 newPos = pos + (m_currentDir * m_speed * dt);
    owner->SetPosition(newPos);

    //==============================
    // 向き（yaw のみ）
    //==============================
    if (m_faceMovement)
    {
        Vector3 flatDir = m_currentDir;
        flatDir.y = 0.0f;

        if (flatDir.LengthSquared() > 1e-6f)
        {
            flatDir.Normalize();
            float yaw = std::atan2(flatDir.x, flatDir.z);

            Vector3 rot = owner->GetRotation();
            rot.y = yaw;
            owner->SetRotation(rot);
        }
    }
}

/// <summary>
/// 向かう地点を配列から取り出す関数
/// </summary>
/// <param name="index"> 欲しい地点の配列番号 </param>
/// <returns>指定した配列番号の地点座標</returns>
Vector3 PatrolComponent::GetPointClamped(int index) const
{
	//サイズを取得
	int count = static_cast<int>(m_waypoints.size());

	//ループなら
	if (m_loop)
	{
		//
		int wrapped = index % count;
		if (wrapped < 0)
		{
			wrapped += count;
		}

		return m_waypoints[wrapped];
	}

	//ループじゃないときは
	int clamped = std::clamp(index, 0, count - 1);
	return m_waypoints[clamped];
}

Vector3 PatrolComponent::EvalCatmullRom(
	const Vector3& p0,
	const Vector3& p1,
	const Vector3& p2,
	const Vector3& p3,
	float t) const
{
	float t2 = t * t;
	float t3 = t2 * t;

	return 0.5f * (
		(2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
		);
}

Vector3 PatrolComponent::EvalCatmullRomTangent(
	const Vector3& p0,
	const Vector3& p1,
	const Vector3& p2,
	const Vector3& p3,
	float t) const
{
	float t2 = t * t;

	return 0.5f * (
		(-p0 + p2) +
		2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t +
		3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t2
		);
}

void PatrolComponent::AdvanceSegment()
{
	int count = static_cast<int>(m_waypoints.size());
	if (count < 2)
	{
		return;
	}

	if (m_loop)
	{
		m_currentIndex = (m_currentIndex + 1) % static_cast<size_t>(count);
		return;
	}

	// 非ループ：最後の区間開始は count-2 まで
	if (m_currentIndex + 1 < static_cast<size_t>(count - 1))
	{
		m_currentIndex++;
		return;
	}

	m_currentIndex = static_cast<size_t>(count - 2);
	m_segmentT = 1.0f;
}

void PatrolComponent::SetWaypoints(const std::vector<Vector3>& pts)
{
	m_waypoints = pts;
	Reset();
}

void PatrolComponent::SetWaypoints(std::vector<Vector3>&& pts)
{
	m_waypoints = std::move(pts);
	Reset();
}

void PatrolComponent::Reset()
{
	m_currentIndex = 0;
	m_segmentT = 0.0f;
	m_splineTension = 0.5f;
}
