#pragma once
#include "global.h"

struct ObjectOBB
{
    std::string name;
    DirectX::BoundingOrientedBox obb;
    std::array<DirectX::XMFLOAT3, 6> normals;
};

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

    // 물체의 모든 면 노멀 벡터 계산
    void CalculateNormals(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& extents, const DirectX::XMFLOAT4& rotation, ObjectOBB& curr);
};

