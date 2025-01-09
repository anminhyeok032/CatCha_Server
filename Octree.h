#pragma once
#include "global.h"

// 옥트리 최대 깊이
constexpr int MAX_DEPTH = 3; 

class OctreeNode {
public:
    DirectX::XMFLOAT3 center;                   // 현재 노드의 중심점 좌표
    float halfSize;                             // 현재 노드의 크기의 절반 (공간 분할의 기준)
    bool activeFlags[8];                        // 8개의 자식 노드 활성 상태
    std::vector<DirectX::XMVECTOR> voxelData;   // 복셀 데이터를 저장하는 벡터
    std::unique_ptr<OctreeNode> children[8];    // 8개의 자식 노드를 저장하는 포인터 배열
    DirectX::BoundingBox boundingBox;           // 현재 노드의 바운딩 박스

    OctreeNode() = default;

    OctreeNode(DirectX::XMFLOAT3 center, float halfSize) : center(center), halfSize(halfSize) 
    {
        std::fill(std::begin(activeFlags), std::end(activeFlags), false);
    }

    // 복셀 데이터를 노드에 삽입하는 함수
    void InsertVoxel(DirectX::XMFLOAT3 voxelPosition, int maxDepth, int currentDepth) 
    {
        // 최대 깊이에 도달하면 복셀 데이터를 현재 노드에 저장
        if (currentDepth >= maxDepth) 
        {
            voxelData.push_back(DirectX::XMLoadFloat3(&voxelPosition));
            return;
        }

        // 복셀의 좌표를 기준으로 어느 자식 노드에 들어갈지 결정 
        int childIndex = 0;
        if (voxelPosition.x > center.x) childIndex |= 1;    // x축 기준으로 오른쪽 영역.
        if (voxelPosition.y > center.y) childIndex |= 2;    // y축 기준으로 위쪽 영역.
        if (voxelPosition.z > center.z) childIndex |= 4;    // z축 기준으로 앞쪽 영역.

        // 해당 자식 노드가 비활성 상태라면 새로운 노드를 생성.
        if (!children[childIndex]) 
        {
            // 새로운 자식 노드의 중심점
            DirectX::XMFLOAT3 childCenter = {
                center.x + ((childIndex & 1) ? halfSize / 2.0f : -halfSize / 2.0f),
                center.y + ((childIndex & 2) ? halfSize / 2.0f : -halfSize / 2.0f),
                center.z + ((childIndex & 4) ? halfSize / 2.0f : -halfSize / 2.0f)
            };

            // 새로운 자식 노드를 생성하고 현재 노드의 자식 배열에 저장
            children[childIndex] = std::make_unique<OctreeNode>(childCenter, halfSize / 2.0f);

            // 새로운 자식 노드의 AABB 최소/최대 좌표 계산
            DirectX::XMFLOAT3 min = {
                childCenter.x - halfSize / 2.0f,
                childCenter.y - halfSize / 2.0f,
                childCenter.z - halfSize / 2.0f
            };
            DirectX::XMFLOAT3 max = {
                childCenter.x + halfSize / 2.0f,
                childCenter.y + halfSize / 2.0f,
                childCenter.z + halfSize / 2.0f
            };
            children[childIndex]->boundingBox = DirectX::BoundingBox(childCenter, { halfSize / 2.0f, halfSize / 2.0f, halfSize / 2.0f });

            // 해당 자식 노드를 활성화
            activeFlags[childIndex] = true;
        }

        // 재귀적으로 자식 노드에 복셀 데이터를 삽입
        children[childIndex]->InsertVoxel(voxelPosition, maxDepth, currentDepth + 1);
    }

    bool RemoveVoxel(const DirectX::BoundingSphere& sphere)
    {
        // 현재 노드의 AABB와 구의 교차 여부 확인
        if (!boundingBox.Intersects(sphere))
        {
            return false;
        }

        // Leaf 노드일 경우
        if (voxelData.empty())
        {
            bool removed = false;
            for (auto it = voxelData.begin(); it != voxelData.end(); ) 
            {
                if (sphere.Contains(*it)) 
                {
                    it = voxelData.erase(it);
                    removed = true;
                }
                else
                {
                    ++it;
                }
            }
            return removed;
        }

        // 하위 노드로 재귀적 탐색
        bool anyChildModified = false;
        for (auto& child : children) 
        {
            if (child && child->RemoveVoxel(sphere)) 
            {
                anyChildModified = true;
            }
        }

        //모든 하위 노드가 비어 있다면 삭제
        if (anyChildModified) 
        {
            for (int i = 0; i < 8; ++i)
            {
                if (children[i] && children[i]->IsEmpty())
                {
                    children[i].reset();       // 자식 노드 삭제
                    activeFlags[i] = false;   // 활성 플래그 비활성화
                }
            }
        }

        return anyChildModified;
    }

    bool IsEmpty() const
    {
        // 복셀 데이터가 있으면 비어 있지 않음
        if (!voxelData.empty())
        {
            return false; 
        }

        // 자식 노드가 있으면 비어 있지 않음
        for (int i = 0; i < 8; ++i)
        {
            if (children[i])
            {
                return false; 
            }
        }

        // 복셀 데이터와 자식 노드가 모두 비었음
        return true; 
    }

    // 노드에 저장된 복셀 데이터를 출력하는 함수
    void PrintNode(int depth = 0) const 
    {
        // 활성화된 자식 노드
        std::vector<int> activeChildIndices;
        for (int i = 0; i < 8; ++i) 
        {
            if (activeFlags[i]) 
            {
                activeChildIndices.push_back(i);
            }
        }

        std::cout << std::string(depth * 3, ' ') << 
            "Depth: " << depth <<
            ", Center: (" << center.x << ", " << center.y << ", " << center.z << 
            "), HalfSize: " << halfSize << 
            ", Voxels: " << voxelData.size() << 
            ", Active Children: " << activeChildIndices.size() <<
            " [";

        for (size_t i = 0; i < activeChildIndices.size(); ++i) 
        {
            std::cout << activeChildIndices[i];
            if (i < activeChildIndices.size() - 1) 
            {
                std::cout << ", ";
            }
        }
        std::cout << "]\n";

        // 활성화된 자식 노드를 재귀적으로 출력
        for (int i = 0; i < 8; ++i) 
        {
            if (children[i]) 
            {
                children[i]->PrintNode(depth + 1);
            }
        }
    }
};
