#include "MapData.h"

std::array<std::array<int, 3>, 12> g_triangle_indices =
{
    // 윗면
    std::array<int, 3>{7, 3, 2}, std::array<int, 3>{7, 2, 6},
    // 아래면
    std::array<int, 3>{1, 4, 5}, std::array<int, 3>{1, 0, 4},
    // 왼쪽면
    std::array<int, 3>{3, 4, 0}, std::array<int, 3>{3, 7, 4},
    // 오른쪽면
    std::array<int, 3>{2, 1, 5}, std::array<int, 3>{2, 5, 6},
    // 앞면
    std::array<int, 3>{3, 1, 2}, std::array<int, 3>{3, 0, 1},
    // 뒷면
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
            CalculateWorldAxes(currentOBB.obb, currentOBB.worldAxes);
            g_obbData.emplace(currentOBB.name, std::move(currentOBB));
            currentOBB = ObjectOBB();  // 다음 객체를 위해 초기화
            continue;
        }

        // "Object Name" 파싱
        if (line.find("Object Name:") != std::string::npos)
        {
            size_t pos = line.find(':');
            currentOBB.name = line.substr(pos + 1);
            currentOBB.name.erase(0, currentOBB.name.find_first_not_of(" \t"));
        }
        // "Position" 파싱
        else if (line.find("Position:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, position);
        }
        // "Rotation" 파싱
        else if (line.find("Rotation:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector4(values, rotation);
        }
        // "Extents" 파싱
        else if (line.find("Extents:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, extents);
            //extents = DirectX::XMFLOAT3(extents.x / 2.0f, extents.y / 2.0f, extents.z / 2.0f);
        }
    }

    // 마지막 객체 추가
    if (!currentOBB.name.empty())
    {
        currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
        CalculateWorldAxes(currentOBB.obb, currentOBB.worldAxes);
        g_obbData.emplace(currentOBB.name, std::move(currentOBB));
    }

    file.close();
    return true;
}

// 라인 Vector3 파싱
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
// 라인 Vector4 파싱
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

// 공백 제거 함수
std::string MapData::Trim(const std::string& str)
{
    const char* whitespace = " \t";
    size_t start = str.find_first_not_of(whitespace);
    size_t end = str.find_last_not_of(whitespace);
    return start == std::string::npos ? "" : str.substr(start, end - start + 1);
}

void MapData::CalculateWorldAxes(DirectX::BoundingOrientedBox& obb, DirectX::XMVECTOR worldAxes[3])
{
    DirectX::XMVECTOR static localAxes[3] = {
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),  // 로컬 X축
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),  // 로컬 Y축
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)   // 로컬 Z축
    };

    // 쿼터니언을 이용해 로컬 축을 회전해 투영할 축을 구해줌
    DirectX::XMVECTOR quaternion = DirectX::XMLoadFloat4(&obb.Orientation);
    // 로컬 축 회전시켜 월드 축구해 저장
    for (int i = 0; i < 3; ++i)
    {
        worldAxes[i] = DirectX::XMVector3Rotate(localAxes[i], quaternion);
    }

}
