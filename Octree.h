#pragma once
#include "global.h"

// 옥트리 최대 깊이
constexpr int MAX_DEPTH = 3; 

class OctreeNode {
public:
    DirectX::XMFLOAT3 center;                               // 현재 노드의 중심점 좌표
    float halfSize;                                         // 현재 노드의 크기의 절반 (공간 분할의 기준)
    bool activeFlags[8];                                    // 8개의 자식 노드 활성 상태
    std::unordered_map<int, DirectX::XMVECTOR> voxelData;   // 복셀 데이터를 저장하는 벡터
    std::unique_ptr<OctreeNode> children[8];                // 8개의 자식 노드를 저장하는 포인터 배열
    DirectX::BoundingBox boundingBox;                       // 현재 노드의 바운딩 박스

    OctreeNode() = default;

    OctreeNode(DirectX::XMFLOAT3 center, float halfSize) : center(center), halfSize(halfSize) 
    {
        std::fill(std::begin(activeFlags), std::end(activeFlags), false);
    }

    // 복사 생성자
    OctreeNode(const OctreeNode& other) {
        center = other.center;
        halfSize = other.halfSize;
        voxelData = other.voxelData;
        boundingBox = other.boundingBox;

        // 자식 노드도 깊은 복사
        for (int i = 0; i < 8; ++i) 
        {
            activeFlags[i] = other.activeFlags[i];
            if (other.children[i]) 
            {
                children[i] = std::make_unique<OctreeNode>(*other.children[i]);
            }
        }
    }

    // 복셀 데이터를 노드에 삽입
    void InsertVoxel(DirectX::XMFLOAT3 voxelPosition, int maxDepth, int currentDepth);
    // 복셀 데이터 충돌 검사 후 삭제
    bool RemoveVoxel(const DirectX::BoundingSphere& sphere);

    // 구와 AABB의 교차 여부 확인
    bool IntersectCheck(const DirectX::BoundingSphere& sphere, DirectX::BoundingBox& AABB) const;

    void DiscoverAABB(const DirectX::BoundingSphere& sphere, std::vector<DirectX::BoundingBox>& result) const;
    
    bool IsEmpty() const;

    // 노드에 저장된 복셀 데이터를 출력
    void PrintNode(int depth = 0) const;
    
};
