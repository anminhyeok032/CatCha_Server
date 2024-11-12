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
    // �� ������ �ε�
    bool LoadMapData(const std::string& filePath);

    // ���� Vector3 �Ľ�
    void ParseVector3(const std::string& values, DirectX::XMFLOAT3& vector);
    // ���� Vector4 �Ľ�
    void ParseVector4(const std::string& values, DirectX::XMFLOAT4& vector);
    // ���� ���� �Լ�
    std::string Trim(const std::string& str);

    // ��ü�� ��� �� ��� ���� ���
    void CalculateNormals(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& extents, const DirectX::XMFLOAT4& rotation, ObjectOBB& curr);
};

