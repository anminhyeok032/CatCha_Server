#pragma once
#include "global.h"

// ��Ʈ�� �ִ� ����
constexpr int MAX_DEPTH = 3; 

class OctreeNode {
public:
    DirectX::XMFLOAT3 center;                               // ���� ����� �߽��� ��ǥ
    float halfSize;                                         // ���� ����� ũ���� ���� (���� ������ ����)
    DirectX::BoundingBox boundingBox;                       // ���� ����� �ٿ�� �ڽ�
    std::bitset<8> activeFlags;                             // 8���� �ڽ� ��� Ȱ�� ����
    std::vector<DirectX::XMVECTOR> voxelData;               // ���� �����͸� �����ϴ� ����
    std::unique_ptr<OctreeNode> children[8];                // 8���� �ڽ� ��带 �����ϴ� ������ �迭


    OctreeNode() = default;

    OctreeNode(DirectX::XMFLOAT3 center, float halfSize) : center(center), halfSize(halfSize) 
    {
        
    }

    // ���� ������
    OctreeNode(const OctreeNode& other) {
        center = other.center;
        halfSize = other.halfSize;
        voxelData = other.voxelData;
        boundingBox = other.boundingBox;

        // �ڽ� ��嵵 ���� ����
        for (int i = 0; i < 8; ++i) 
        {
            activeFlags[i] = other.activeFlags[i];
            if (other.children[i]) 
            {
                children[i] = std::make_unique<OctreeNode>(*other.children[i]);
            }
        }
    }

    // ���� �����͸� ��忡 ����
    void InsertVoxel(DirectX::XMFLOAT3 voxelPosition, int maxDepth, int currentDepth);
    // ���� ������ �浹 �˻� �� ����
    bool RemoveVoxel(const DirectX::BoundingSphere& sphere);

    // ���� AABB�� ���� ���� Ȯ��
    bool IntersectCheck(const DirectX::BoundingSphere& sphere, DirectX::BoundingBox& AABB) const;

    void DiscoverAABB(const DirectX::BoundingSphere& sphere, std::vector<DirectX::BoundingBox>& result) const;
    
    bool IsEmpty() const;

    // ��忡 ����� ���� �����͸� ���
    void PrintNode(int depth = 0) const;
    
};
