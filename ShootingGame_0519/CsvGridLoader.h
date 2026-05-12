#pragma once
#include <vector>
#include <string>

class CsvGridLoader
{
public:
    static std::vector<std::vector<int>> LoadGrid(const std::string& filePath);
};
