#include "Octree.h"

UINT vexel_count = 0;

void OctreeNode::InsertVoxel(DirectX::XMFLOAT3 voxelPosition, int maxDepth, int currentDepth)
{
    // 최대 깊이에 도달하면 복셀 데이터를 현재 노드에 저장
    if (currentDepth >= maxDepth)
    {
        voxelData[vexel_count++] = (DirectX::XMLoadFloat3(&voxelPosition));
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

bool OctreeNode::RemoveVoxel(const DirectX::BoundingSphere& sphere)
{
    // 현재 노드의 AABB와 구의 교차 여부 확인
    if (false == boundingBox.Intersects(sphere))
    {
        return false;
    }

    // Leaf 노드일 경우
    if (false == voxelData.empty())
    {
        bool removed = false;
        for (auto it = voxelData.begin(); it != voxelData.end(); )
        {
            if (sphere.Contains(it->second))
            {
                it = voxelData.erase(it);
                removed = true;
            }
            else
            {
                ++it; // 조건이 충족되지 않으면 다음으로 이동
            }
        }
        return removed; // 복셀 삭제 여부 반환
    }

    // 하위 노드로 재귀적 탐색
    bool is_child_changed = false;
    for (auto& child : children)
    {
        if (child && child->RemoveVoxel(sphere))
        {
            is_child_changed = true;
        }
    }

    // 모든 하위 노드가 비어 있다면 삭제
    if (true == is_child_changed)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (children[i] && children[i]->IsEmpty())
            {
                children[i].reset();        // 자식 노드 삭제
                activeFlags[i] = false;     // 활성 플래그 비활성화
            }
        }
    }

    return is_child_changed;
}

bool OctreeNode::IsEmpty() const
{
    // 복셀 데이터가 있으면 비어 있지 않음
    if (false == voxelData.empty())
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

void OctreeNode::PrintNode(int depth) const
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