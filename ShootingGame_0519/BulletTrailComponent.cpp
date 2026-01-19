#include "BulletTrailComponent.h"
#include "Renderer.h"
#include <algorithm>

void BulletTrailComponent::Initialize()
{
    m_elapsed = 0.0f;
    m_isFinished = false;
}

void BulletTrailComponent::SetSegment(const DirectX::SimpleMath::Vector3& startPos, const DirectX::SimpleMath::Vector3& endPos)
{
    m_startPos = startPos;
    m_endPos = endPos;
}

void BulletTrailComponent::Update(float dt)
{
    if (dt <= 0.0f)
    {
        return;
    }

    if (m_isFinished)
    {
        return;
    }

    m_elapsed += dt;

    if (m_duration <= 0.0f)
    {
        m_isFinished = true;
        return;
    }

    if (m_elapsed >= m_duration)
    {
        m_isFinished = true;
    }
}

void BulletTrailComponent::Draw(float dt)
{
    if (m_isFinished)
    {
        return;
    }

    if (!m_textureSrv)
    {
        return;
    }

    // フェードアウト（時間が進むほど薄く）
    float t = (m_duration > 0.0f) ? std::clamp(m_elapsed / m_duration, 0.0f, 1.0f) : 1.0f;
    DirectX::SimpleMath::Vector4 col = m_color;
    col.w *= (1.0f - t);

    Renderer::DrawTrailBillboard(
        m_textureSrv,
        m_startPos,
        m_endPos,
        m_width,
        col,
        m_isAdditive);
}
