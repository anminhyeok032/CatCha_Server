#include "MapData.h"

std::array<std::array<int, 3>, 12> g_triangle_indices =
{
    // À­¸é
    //std::array<int, 3>{0, 1, 5}, std::array<int, 3>{0, 5, 4},
    //// ¾Æ·¡¸é
    //std::array<int, 3>{2, 3, 7}, std::array<int, 3>{2, 7, 6},
    //// ¿ŞÂÊ¸é
    //std::array<int, 3>{0, 3, 7}, std::array<int, 3>{0, 7, 4},
    //// ¿À¸¥ÂÊ¸é
    //std::array<int, 3>{1, 2, 6}, std::array<int, 3>{1, 6, 5},
    //// ¾Õ¸é
    //std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3},
    //// µŞ¸é
    //std::array<int, 3>{4, 5, 6}, std::array<int, 3>{4, 6, 7}

    // À­¸é
    //std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3},
    //// ¾Æ·¡¸é
    //std::array<int, 3>{4, 6, 5}, std::array<int, 3>{4, 7, 6},
    //// ¿ŞÂÊ¸é
    //std::array<int, 3>{0, 7, 4}, std::array<int, 3>{0, 3, 7},
    //// ¿À¸¥ÂÊ¸é
    //std::array<int, 3>{1, 5, 6}, std::array<int, 3>{1, 6, 2},
    //// ¾Õ¸é
    //std::array<int, 3>{3, 2, 7}, std::array<int, 3>{3, 6, 7},
    //// µŞ¸é
    //std::array<int, 3>{0, 4, 1}, std::array<int, 3>{1, 4, 5}

// À­¸é
std::array<int, 3>{7, 3, 2}, std::array<int, 3>{7, 2, 6},
// ¾Æ·¡¸é
std::array<int, 3>{1, 4, 5}, std::array<int, 3>{1, 4, 0},
// ¿ŞÂÊ¸é
std::array<int, 3>{3, 4, 0}, std::array<int, 3>{3, 7, 4},
// ¿À¸¥ÂÊ¸é
std::array<int, 3>{2, 1, 5}, std::array<int, 3>{2, 5, 6},
// ¾Õ¸é
std::array<int, 3>{3, 1, 2}, std::array<int, 3>{3, 0, 1},
// µŞ¸é
std::array<int, 3>{7, 6, 5}, std::array<int, 3>{7, 5, 4}
};



bool MapData::LoadMapData(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    ObjectOBB currentOBB;
    DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
            g_obbData.emplace(currentOBB.name, std::move(currentOBB));
            currentOBB = ObjectOBB();  // ´ÙÀ½ °´Ã¼¸¦ À§ÇØ ÃÊ±âÈ­
            continue;
        }

        // "Object Name" ÆÄ½Ì
        if (line.find("Object Name:") != std::string::npos)
        {
            size_t pos = line.find(':');
            currentOBB.name = line.substr(pos + 1);
            currentOBB.name.erase(0, currentOBB.name.find_first_not_of(" \t"));
        }
        // "Position" ÆÄ½Ì
        else if (line.find("Position:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, position);
        }
        // "Rotation" ÆÄ½Ì
        else if (line.find("Rotation:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector4(values, rotation);
        }
        // "Extents" ÆÄ½Ì
        else if (line.find("Extents:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, extents);
            extents = DirectX::XMFLOAT3(extents.x / 2.0f, extents.y / 2.0f, extents.z / 2.0f);
        }
    }

    // ¸¶Áö¸· °´Ã¼ Ãß°¡
    if (!currentOBB.name.empty())
    {
        currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
        g_obbData.emplace(currentOBB.name, std::move(currentOBB));
    }

    file.close();
    return true;
}

// ¶óÀÎ Vector3 ÆÄ½Ì
void MapData::ParseVector3(const std::string& values, DirectX::XMFLOAT3& vector)
{
    std::istringstream iss(values);
    std::string token;

    std::getline(iss, token, ',');
    vector.x = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.y = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.z = std::stof(Trim(token));
}
// ¶óÀÎ Vector4 ÆÄ½Ì
void MapData::ParseVector4(const std::string& values, DirectX::XMFLOAT4& vector)
{
    std::istringstream iss(values);
    std::string token;

    std::getline(iss, token, ',');
    vector.x = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.y = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.z = std::stof(Trim(token));

    std::getline(iss, token, ',');
    vector.w = std::stof(Trim(token));
}

// °ø¹é Á¦°Å ÇÔ¼ö
std::string MapData::Trim(const std::string& str)
{
    const char* whitespace = " \t";
    size_t start = str.find_first_not_of(whitespace);
    size_t end = str.find_last_not_of(whitespace);
    return start == std::string::npos ? "" : str.substr(start, end - start + 1);
}
