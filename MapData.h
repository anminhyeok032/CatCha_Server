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
    // 맵 데이터 로드
    bool LoadMapData(const std::string& filePath);

    // 라인 Vector3 파싱
    void ParseVector3(const std::string& values, DirectX::XMFLOAT3& vector);
    // 라인 Vector4 파싱
    void ParseVector4(const std::string& values, DirectX::XMFLOAT4& vector);
    // 공백 제거 함수
    std::string Trim(const std::string& str);
    // 월드 좌표 축 계산
    void CalculateWorldAxes(DirectX::BoundingOrientedBox& obb, DirectX::XMVECTOR worldAxes[3]);
    // AI를 위한 타일맵 생성
    void CheckTileMap4AI();
    // 타일맵 출력
    void PrintTileMap();
};

