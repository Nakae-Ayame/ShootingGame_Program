#include "StageRouteBuilder.h"

bool StageRouteBuilder::IsInside(int row, int col, int rowCount, int colCount)
{
    if (row < 0)
    {
        return false;
    }

    if (col < 0)
    {
        return false;
    }

    if (row >= rowCount)
    {
        return false;
    }

    if (col >= colCount)
    {
        return false;
    }

    return true;
}

std::vector<Vector3> StageRouteBuilder::BuildRoute(
    const std::vector<std::vector<int>>& grid,
    float cellSize
)
{
    std::vector<Vector3> waypoints;

    if (grid.empty())
    {
        return waypoints;
    }

    int rowCount = static_cast<int>(grid.size());
    int colCount = static_cast<int>(grid[0].size());
    int centerCol = colCount / 2;

    int startRow = -1;
    int startCol = -1;

    for (int row = 0; row < rowCount; row++)
    {
        for (int col = 0; col < colCount; col++)
        {
            if (grid[row][col] == 3)
            {
                startRow = row;
                startCol = col;
                break;
            }
        }

        if (startRow != -1)
        {
            break;
        }
    }

    if (startRow == -1)
    {
        return waypoints;
    }

    std::vector<std::vector<bool>> visited;
    visited.resize(rowCount);

    for (int row = 0; row < rowCount; row++)
    {
        visited[row].resize(colCount, false);
    }

    int currentRow = startRow;
    int currentCol = startCol;

    while (true)
    {
        visited[currentRow][currentCol] = true;

        float x = static_cast<float>(currentCol - centerCol) * cellSize;
        float y = 0.0f;
        float z = static_cast<float>(currentRow) * cellSize;

        waypoints.push_back(Vector3(x, y, z));

        bool foundNext = false;
        int nextRow = currentRow;
        int nextCol = currentCol;

        const int dirRow[4] = { -1, 1, 0, 0 };
        const int dirCol[4] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; i++)
        {
            int checkRow = currentRow + dirRow[i];
            int checkCol = currentCol + dirCol[i];

            if (!IsInside(checkRow, checkCol, rowCount, colCount))
            {
                continue;
            }

            if (visited[checkRow][checkCol])
            {
                continue;
            }

            if (grid[checkRow][checkCol] == 9)
            {
                nextRow = checkRow;
                nextCol = checkCol;
                foundNext = true;
                break;
            }
        }

        if (!foundNext)
        {
            break;
        }

        currentRow = nextRow;
        currentCol = nextCol;
    }

    return waypoints;
}