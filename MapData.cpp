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

    g_obbData.clear();

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
            CalculateWorldAxes(currentOBB.obb, currentOBB.worldAxes);
            if (currentOBB.name != "Escape")
            {
                g_obbData.emplace_back(std::move(currentOBB));
            }
            else
            {
                g_EscapeOBB = currentOBB.obb;
            }
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
        if (currentOBB.name != "Escape")
        {
            g_obbData.emplace_back(std::move(currentOBB));
        }
        else
        {
            g_EscapeOBB = currentOBB.obb;
        }
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

void MapData::CheckTileMap4AI()
{
    int numTilesX = TILE_MAP_WIDTH / TILE_SIZE;
    int numTilesZ = TILE_MAP_LENGTH / TILE_SIZE;

    g_tile_map.reserve(numTilesX * numTilesZ);

    for (int x = 0; x < numTilesX; ++x) 
    {
        for (int z = 0; z < numTilesZ; ++z) 
        {
            g_tile_map.push_back({ x, z, true });
        }
    }

    for (const auto& object : g_obbData)
    {
        if (object.name == "Ground-0_OBB") continue;

        for (auto& tile : g_tile_map)
        {
            if (false == tile.walkable) continue;

            // 타일의 중심 좌표
            float tile_center_x = -TILE_MAP_WIDTH / 2.0f + tile.x * TILE_SIZE + TILE_SIZE / 2.0f;
            float tile_center_z = -TILE_MAP_LENGTH / 2.0f + tile.z * TILE_SIZE + TILE_SIZE / 2.0f;

            // 타일의 AABB와 물건의 OBB 충돌 체크 후 walkable 설정
            DirectX::BoundingBox tile_aabb(
                DirectX::XMFLOAT3(tile_center_x, FLOOR_Y, tile_center_z),
                DirectX::XMFLOAT3(TILE_SIZE / 2.0f, 2.5f, TILE_SIZE / 2.0f));

            if (true == object.obb.Intersects(tile_aabb))
            {
                tile.walkable = false;
            }
        }
    }
    // walkable이 true인 타일만 담아 사용
    BuildWalkableTileMapIndex();
}

void MapData::BuildWalkableTileMapIndex()
{
    g_tile_map_walkable_only.clear();
    int numTilesX = TILE_MAP_WIDTH / TILE_SIZE;
    int numTilesZ = TILE_MAP_LENGTH / TILE_SIZE;
    for (int i = 0; i < g_tile_map.size(); ++i)
    {
        if (true == g_tile_map[i].walkable)
        {
            g_tile_map_walkable_only.emplace_back(i);
        }
    }
}


void MapData::PrintTileMap()
{
    int numTilesX = TILE_MAP_WIDTH / TILE_SIZE;
    int numTilesZ = TILE_MAP_LENGTH / TILE_SIZE;

    for (int z = 0; z < numTilesZ; ++z)
    {
        for (int x = 0; x < numTilesX; ++x)
        {
            // 타일맵의 1차원 인덱스 계산
            const Tile& tile = g_tile_map[x * numTilesZ + z];

            // walkable 여부에 따라 출력
            if (true == tile.walkable)
            {
                std::cout << ". ";
            }
            else
            {
                std::cout << "X ";
            }
        }
        std::cout << std::endl;
    }
}