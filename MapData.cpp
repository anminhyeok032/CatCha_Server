#include "MapData.h"

std::array<std::array<int, 3>, 12> g_triangle_indices =
{
    std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3},		// �Ʒ���
    std::array<int, 3>{4, 5, 6}, std::array<int, 3>{4, 6, 7},		// ����
    std::array<int, 3>{0, 1, 5}, std::array<int, 3>{0, 5, 4},		// �ո�
    std::array<int, 3>{2, 3, 7}, std::array<int, 3>{2, 7, 6},		// �޸�
    std::array<int, 3>{0, 3, 7}, std::array<int, 3>{0, 7, 4},		// ���ʸ�
    std::array<int, 3>{1, 2, 6}, std::array<int, 3>{1, 6, 5}		// �����ʸ�
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
            CalculateNormals(position, extents, rotation, currentOBB);
            g_obbData.emplace(currentOBB.name, std::move(currentOBB));
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
            //position = MathHelper::Multiply(position, 50.0f);
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
            //extents = MathHelper::Multiply(extents, 50.0f);
        }
    }

    // ������ ��ü �߰�
    if (!currentOBB.name.empty())
    {
        currentOBB.obb = DirectX::BoundingOrientedBox(position, extents, rotation);
        CalculateNormals(position, extents, rotation, currentOBB);
        g_obbData.emplace(currentOBB.name, std::move(currentOBB));
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

// ��ü�� ��� �� ��� ���� ���
void MapData::CalculateNormals(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& extents, const DirectX::XMFLOAT4& rotation, ObjectOBB& curr)
{
    // �� ���� ����
    DirectX::XMVECTOR extentX = DirectX::XMVectorSet(extents.x, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR extentY = DirectX::XMVectorSet(0.0f, extents.y, 0.0f, 0.0f);
    DirectX::XMVECTOR extentZ = DirectX::XMVectorSet(0.0f, 0.0f, extents.z, 0.0f);

    // ȸ�� ��� ����
    DirectX::XMVECTOR quatRotation = DirectX::XMLoadFloat4(&rotation);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(quatRotation);

    // �� ��� ���� ���
    DirectX::XMVECTOR normalX = DirectX::XMVector3Normalize(XMVector3TransformNormal(extentX, rotationMatrix));
    DirectX::XMVECTOR normalY = DirectX::XMVector3Normalize(XMVector3TransformNormal(extentY, rotationMatrix));
    DirectX::XMVECTOR normalZ = DirectX::XMVector3Normalize(XMVector3TransformNormal(extentZ, rotationMatrix));

    // +X, -X ���� ���
    DirectX::XMStoreFloat3(&curr.normals[0], normalX);                              // +X ��
    DirectX::XMStoreFloat3(&curr.normals[1], DirectX::XMVectorNegate(normalX));     // -X ��

    // +Y, -Y ���� ���
    DirectX::XMStoreFloat3(&curr.normals[2], normalY);                              // +Y ��
    DirectX::XMStoreFloat3(&curr.normals[3], DirectX::XMVectorNegate(normalY));     // -Y ��

    // +Z, -Z ���� ���
    DirectX::XMStoreFloat3(&curr.normals[4], normalZ);                              // +Z ��
    DirectX::XMStoreFloat3(&curr.normals[5], DirectX::XMVectorNegate(normalZ));     // -Z ��
}