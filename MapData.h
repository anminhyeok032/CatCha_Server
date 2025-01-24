#pragma once
#include "global.h"

extern std::unordered_map<std::string, ObjectOBB> g_obbData;
extern std::array<std::array<int, 3>, 12> g_triangle_indices;
extern std::vector<Tile> g_tile_map;

constexpr int TILE_SIZE = 10;
constexpr int TILE_MAP_WIDTH = 1200;
constexpr int TILE_MAP_LENGTH = 1200;

class MapData {
public:
    // �� ������ �ε�
    bool LoadMapData(const std::string& filePath);

    // ���� Vector3 �Ľ�
    void ParseVector3(const std::string& values, DirectX::XMFLOAT3& vector);
    // ���� Vector4 �Ľ�
    void ParseVector4(const std::string& values, DirectX::XMFLOAT4& vector);
    // ���� ���� �Լ�
    std::string Trim(const std::string& str);
    // ���� ��ǥ �� ���
    void CalculateWorldAxes(DirectX::BoundingOrientedBox& obb, DirectX::XMVECTOR worldAxes[3]);
    // AI�� ���� Ÿ�ϸ� ����
    void CheckTileMap4AI();
    // Ÿ�ϸ� ���
    void PrintTileMap();
};

