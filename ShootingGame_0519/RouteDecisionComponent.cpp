#include "RouteDecisionComponent.h"
#include "PatrolComponent.h"
#include "GameObject.h"
#include <algorithm>
#include <numeric>

void RouteDecisionComponent::Initialize()
{
    std::random_device rd;
    m_rng = std::mt19937(rd());

    m_isBranching = false;
    m_activeBranchPointIndex = 0;
    m_resumeMainIndex = 0;
    m_cooldownTimer = 0.0f;
    m_editingBranchPoint = -1;
}

void RouteDecisionComponent::Update(float dt)
{
    if (!GetOwner())
    {
        return;
    }

    if (dt <= 0.0f)
    {
        return;
    }

    if (!m_patrol)
    {
        return;
    }

    if (m_mainWaypoints.size() < 2)
    {
        return;
    }

    if (m_cooldownTimer > 0.0f)
    {
        m_cooldownTimer -= dt;
        if (m_cooldownTimer < 0.0f)
        {
            m_cooldownTimer = 0.0f;
        }
    }

    if (m_isBranching)
    {
        if (IsBranchRouteFinished())
        {
            ExitBranch();
        }
        return;
    }

    TryEnterBranch();
}

void RouteDecisionComponent::SetPatrol(PatrolComponent* patrol)
{
    m_patrol = patrol;
}

void RouteDecisionComponent::SetMainWaypoints(const std::vector<Vector3>& pts)
{
    m_mainWaypoints = pts;

    if (m_patrol)
    {
        m_patrol->SetWaypoints(m_mainWaypoints);
    }
}

void RouteDecisionComponent::SetMainWaypoints(std::vector<Vector3>&& pts)
{
    m_mainWaypoints = std::move(pts);

    if (m_patrol)
    {
        m_patrol->SetWaypoints(m_mainWaypoints);
    }
}

void RouteDecisionComponent::SetArrivalThreshold(float t)
{
    m_arrivalThreshold = t;
}

void RouteDecisionComponent::SetBranchCooldown(float sec)
{
    m_branchCooldown = sec;
}

void RouteDecisionComponent::AddBranchPoint(size_t mainIndex)
{
    BranchPoint bp;
    bp.mainIndex = mainIndex;

    m_branchPoints.push_back(bp);
    m_editingBranchPoint = static_cast<int>(m_branchPoints.size()) - 1;
}

void RouteDecisionComponent::AddBranchOption(const std::vector<Vector3>& loopPts, float weight)
{
    if (m_editingBranchPoint < 0)
    {
        return;
    }

    if (weight <= 0.0f)
    {
        weight = 1.0f;
    }

    BranchOption opt;
    opt.loopWaypoints = loopPts;
    opt.weight = weight;

    m_branchPoints[m_editingBranchPoint].options.push_back(opt);
}

void RouteDecisionComponent::AddBranchOption(std::vector<Vector3>&& loopPts, float weight)
{
    if (m_editingBranchPoint < 0)
    {
        return;
    }

    if (weight <= 0.0f)
    {
        weight = 1.0f;
    }

    BranchOption opt;
    opt.loopWaypoints = std::move(loopPts);
    opt.weight = weight;

    m_branchPoints[m_editingBranchPoint].options.push_back(opt);
}

void RouteDecisionComponent::ClearBranchPoints()
{
    m_branchPoints.clear();
    m_editingBranchPoint = -1;
}

const RouteDecisionComponent::BranchPoint* RouteDecisionComponent::FindBranchPointByMainIndex(size_t mainIndex) const
{
    for (const auto& bp : m_branchPoints)
    {
        if (bp.mainIndex == mainIndex)
        {
            return &bp;
        }
    }
    return nullptr;
}

int RouteDecisionComponent::FindBranchPointIndexByMainIndex(size_t mainIndex) const
{
    for (int i = 0; i < static_cast<int>(m_branchPoints.size()); i++)
    {
        if (m_branchPoints[i].mainIndex == mainIndex)
        {
            return i;
        }
    }
    return -1;
}

void RouteDecisionComponent::TryEnterBranch()
{
    if (m_cooldownTimer > 0.0f)
    {
        return;
    }

    size_t curIndex = m_patrol->GetCurrentIndex();
    int bpIndex = FindBranchPointIndexByMainIndex(curIndex);
    if (bpIndex < 0)
    {
        return;
    }

    const BranchPoint& bp = m_branchPoints[bpIndex];
    if (bp.options.empty())
    {
        return;
    }

    if (m_lastTriggeredMainIndex == bp.mainIndex)
    {
        // 同じ地点に戻った直後の連続分岐防止（クールダウンでも守る）
        return;
    }

    // “本当に分岐地点に近いか” を位置で最終確認（スプライン/ステアリングで index だけだと誤発火するため）
    Vector3 pos = GetOwner()->GetPosition();

    if (bp.mainIndex >= m_mainWaypoints.size())
    {
        return;
    }

    Vector3 junctionPos = m_mainWaypoints[bp.mainIndex];

    if (!IsCloseTo(pos, junctionPos, m_arrivalThreshold))
    {
        return;
    }

    EnterBranch(static_cast<size_t>(bpIndex));
}

void RouteDecisionComponent::EnterBranch(size_t branchPointIndex)
{
    if (branchPointIndex >= m_branchPoints.size())
    {
        return;
    }

    const BranchPoint& bp = m_branchPoints[branchPointIndex];
    if (bp.options.empty())
    {
        return;
    }

    size_t optionIndex = ChooseOptionIndex(bp);
    if (optionIndex >= bp.options.size())
    {
        return;
    }

    if (bp.mainIndex >= m_mainWaypoints.size())
    {
        return;
    }

    m_activeBranchPointIndex = branchPointIndex;
    m_activeJunctionPos = m_mainWaypoints[bp.mainIndex];

    // 分岐終了後は “基本は次の本線” から再開（無限分岐しにくい）
    m_resumeMainIndex = bp.mainIndex + 1;
    if (m_resumeMainIndex >= m_mainWaypoints.size())
    {
        m_resumeMainIndex = m_mainWaypoints.size() - 1;
    }

    m_activeBranchRoute = BuildLoopRoute(m_activeJunctionPos, bp.options[optionIndex].loopWaypoints);

    // 分岐ルートは “非ループ” にして終端検知しやすくする
    m_patrol->SetLoop(false);
    m_patrol->SetPingPong(false);
    m_patrol->SetWaypoints(m_activeBranchRoute);

    m_isBranching = true;
    m_lastTriggeredMainIndex = bp.mainIndex;
    m_cooldownTimer = m_branchCooldown;
}

void RouteDecisionComponent::ExitBranch()
{
    // 本線に戻す（本線はループ運用が多い想定）
    m_patrol->SetWaypoints(m_mainWaypoints);
    m_patrol->SetLoop(true);

    m_activeBranchRoute.clear();
    m_isBranching = false;
    m_cooldownTimer = m_branchCooldown;
}

bool RouteDecisionComponent::IsCloseTo(const Vector3& a, const Vector3& b, float threshold) const
{
    Vector3 d = a - b;
    float distSq = d.LengthSquared();
    return distSq <= (threshold * threshold);
}

size_t RouteDecisionComponent::ChooseOptionIndex(const BranchPoint& bp)
{
    // weight による重み付きランダム
    float total = 0.0f;
    for (const auto& opt : bp.options)
    {
        if (opt.weight > 0.0f)
        {
            total += opt.weight;
        }
    }

    if (total <= 0.0f)
    {
        return 0;
    }

    std::uniform_real_distribution<float> dist(0.0f, total);
    float r = dist(m_rng);

    float acc = 0.0f;
    for (size_t i = 0; i < bp.options.size(); i++)
    {
        float w = bp.options[i].weight;
        if (w <= 0.0f)
        {
            continue;
        }

        acc += w;
        if (r <= acc)
        {
            return i;
        }
    }

    return 0;
}

std::vector<Vector3> RouteDecisionComponent::BuildLoopRoute(const Vector3& junctionPos, const std::vector<Vector3>& loopPts) const
{
    std::vector<Vector3> out;

    // 先頭は分岐地点
    out.push_back(junctionPos);

    // 途中点
    for (const auto& p : loopPts)
    {
        out.push_back(p);
    }

    // 最後は分岐地点に戻る（仕様）
    out.push_back(junctionPos);

    return out;
}

bool RouteDecisionComponent::IsBranchRouteFinished() const
{
    if (!m_patrol)
    {
        return false;
    }

    if (m_activeBranchRoute.size() < 2)
    {
        return false;
    }

    size_t endSegmentIndex = m_activeBranchRoute.size() - 2;
    size_t curIndex = m_patrol->GetCurrentIndex();

    if (curIndex != endSegmentIndex)
    {
        return false;
    }

    Vector3 pos = GetOwner()->GetPosition();
    if (!IsCloseTo(pos, m_activeJunctionPos, m_arrivalThreshold))
    {
        return false;
    }

    return true;
}
