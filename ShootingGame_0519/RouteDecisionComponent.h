#pragma once
#include "Component.h"
#include <vector>
#include <random>
#include <cstddef>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class PatrolComponent;

class RouteDecisionComponent : public Component
{
public:
    RouteDecisionComponent() = default;
    ~RouteDecisionComponent() override = default;

    void Initialize() override;
    void Update(float dt) override;

    //--------Set関数-------
    void SetPatrol(PatrolComponent* patrol);
    void SetMainWaypoints(const std::vector<Vector3>& pts);
    void SetMainWaypoints(std::vector<Vector3>&& pts);
    void SetArrivalThreshold(float t);
    void SetBranchCooldown(float sec);

    // 分岐地点の追加（本線の index を指定）
    void AddBranchPoint(size_t mainIndex);

    // 直近に AddBranchPoint() した分岐地点へ、候補ループを追加（最大3想定）
    // loopPts は「分岐地点から出て戻るまで」の“途中点”だけでもOK（最後に junction を自動で足す）
    void AddBranchOption(const std::vector<Vector3>& loopPts, float weight = 1.0f);
    void AddBranchOption(std::vector<Vector3>&& loopPts, float weight = 1.0f);

    void ClearBranchPoints();

    //--------Get関数-------
    bool IsBranching() const { return m_isBranching; }

private:
    struct BranchOption
    {
        std::vector<Vector3> loopWaypoints;
        float weight = 1.0f;
    };

    struct BranchPoint
    {
        size_t mainIndex = 0;
        std::vector<BranchOption> options; // 最大3想定
    };

private:
    //--------------参照関連------------------
    PatrolComponent* m_patrol = nullptr;

    //--------------本線関連------------------
    std::vector<Vector3> m_mainWaypoints;

    //--------------分岐関連------------------
    std::vector<BranchPoint> m_branchPoints;
    int m_editingBranchPoint = -1;

    bool m_isBranching = false;
    size_t m_activeBranchPointIndex = 0;
    size_t m_resumeMainIndex = 0;

    // 分岐開始地点（座標）
    Vector3 m_activeJunctionPos = Vector3(0.0f, 0.0f, 0.0f);

    // 今走っている分岐ルート（完了検知に使う）
    std::vector<Vector3> m_activeBranchRoute;

    //--------------判定/クールダウン関連------------------
    float m_arrivalThreshold = 0.5f;
    float m_branchCooldown = 0.25f;
    float m_cooldownTimer = 0.0f;
    size_t m_lastTriggeredMainIndex = static_cast<size_t>(-1);

    //--------------乱数関連------------------
    std::mt19937 m_rng;

private:
    const BranchPoint* FindBranchPointByMainIndex(size_t mainIndex) const;
    int FindBranchPointIndexByMainIndex(size_t mainIndex) const;

    void TryEnterBranch();
    void EnterBranch(size_t branchPointIndex);
    void ExitBranch();

    bool IsCloseTo(const Vector3& a, const Vector3& b, float threshold) const;

    size_t ChooseOptionIndex(const BranchPoint& bp);
    std::vector<Vector3> BuildLoopRoute(const Vector3& junctionPos, const std::vector<Vector3>& loopPts) const;

    bool IsBranchRouteFinished() const;
};
