#include "MapData.h"

std::array<std::array<int, 3>, 12> g_triangle_indices =
{
    // ����
    std::array<int, 3>{7, 3, 2}, std::array<int, 3>{7, 2, 6},
    // �Ʒ���
    std::array<int, 3>{1, 4, 5}, std::array<int, 3>{1, 0, 4},
    // ���ʸ�
    std::array<int, 3>{3, 4, 0}, std::array<int, 3>{3, 7, 4},
    // �����ʸ�
    std::array<int, 3>{2, 1, 5}, std::array<int, 3>{2, 5, 6},
    // �ո�
    std::array<int, 3>{3, 1, 2}, std::array<int, 3>{3, 0, 1},
    // �޸�
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
            currentOBB = ObjectOBB();  // ���� ��ü�� ���� �ʱ�ȭ
            continue;
        }

        // "Object Name" �Ľ�
        if (line.find("Object Name:") != std::string::npos)
        {
            size_t pos = line.find(':');
            currentOBB.name = line.substr(pos + 1);
            currentOBB.name.erase(0, currentOBB.name.find_first_not_of(" \t"));
        }
        // "Position" �Ľ�
        else if (line.find("Position:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, position);
        }
        // "Rotation" �Ľ�
        else if (line.find("Rotation:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector4(values, rotation);
        }
        // "Extents" �Ľ�
        else if (line.find("Extents:") != std::string::npos)
        {
            size_t pos = line.find(':');
            std::string values = line.substr(pos + 1);
            ParseVector3(values, extents);
            //extents = DirectX::XMFLOAT3(extents.x / 2.0f, extents.y / 2.0f, extents.z / 2.0f);
        }
    }

    // ������ ��ü �߰�
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

// ���� Vector3 �Ľ�
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
// ���� Vector4 �Ľ�
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

// ���� ���� �Լ�
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
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),  // ���� X��
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),  // ���� Y��
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)   // ���� Z��
    };

    // ���ʹϾ��� �̿��� ���� ���� ȸ���� ������ ���� ������
    DirectX::XMVECTOR quaternion = DirectX::XMLoadFloat4(&obb.Orientation);
    // ���� �� ȸ������ ���� �౸�� ����
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

            // Ÿ���� �߽� ��ǥ
            float tile_center_x = -TILE_MAP_WIDTH / 2.0f + tile.x * TILE_SIZE + TILE_SIZE / 2.0f;
            float tile_center_z = -TILE_MAP_LENGTH / 2.0f + tile.z * TILE_SIZE + TILE_SIZE / 2.0f;

            // Ÿ���� AABB�� ������ OBB �浹 üũ �� walkable ����
            DirectX::BoundingBox tile_aabb(
                DirectX::XMFLOAT3(tile_center_x, FLOOR_Y, tile_center_z),
                DirectX::XMFLOAT3(TILE_SIZE / 2.0f, 2.5f, TILE_SIZE / 2.0f));

            if (true == object.obb.Intersects(tile_aabb))
            {
                tile.walkable = false;
            }
        }
    }
    // walkable�� true�� Ÿ�ϸ� ��� ���
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
            // Ÿ�ϸ��� 1���� �ε��� ���
            const Tile& tile = g_tile_map[x * numTilesZ + z];

            // walkable ���ο� ���� ���
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