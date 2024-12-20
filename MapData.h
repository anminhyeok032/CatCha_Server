#pragma once
#include "global.h"

extern std::unordered_map<std::string, ObjectOBB> g_obbData;
extern std::array<std::array<int, 3>, 12> g_triangle_indices;

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
};

