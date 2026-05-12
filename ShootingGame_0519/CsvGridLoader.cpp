#include "CsvGridLoader.h"
#include <fstream>
#include <sstream>

std::vector<std::vector<int>> CsvGridLoader::LoadGrid(const std::string& filePath)
{
    std::vector<std::vector<int>> grid;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return grid;
    }

    std::string line;

    //ファイルを1行ずつ読む
    while (std::getline(file, line))
    {
        std::vector<int> row;

        std::stringstream ss(line); 
        std::string cell;

        //行をカンマで分割
        while (std::getline(ss, cell, ','))
        {
            //空白削除
            cell.erase(0, cell.find_first_not_of(" \t\r\n"));
            cell.erase(cell.find_last_not_of(" \t\r\n") + 1);

            if (cell.empty())
            {
                row.push_back(0);
                continue;
            }

            try
            {
                row.push_back(std::stoi(cell));
            }
            catch (...)
            {
                row.push_back(0);
            }
        }

        grid.push_back(row);
    }

    return grid;
}