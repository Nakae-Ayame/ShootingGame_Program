#pragma once
#include <vector>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class StageRouteBuilder
{
public:
    static std::vector<Vector3> BuildRoute(
        const std::vector<std::vector<int>>& grid,
        float cellSize
    );

private:
    static bool IsInside(
        int row,
        int col,
        int rowCount,
        int colCount
    );
};