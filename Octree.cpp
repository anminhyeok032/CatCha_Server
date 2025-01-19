#include "Octree.h"

UINT vexel_count = 0;

const DirectX::XMFLOAT3 CHEESE_POS[CHEESE_NUM] =
{
    DirectX::XMFLOAT3(169.475f, 10.049f, 230.732f),
    DirectX::XMFLOAT3(169.475f, 10.049f, 270.732f),
    DirectX::XMFLOAT3(-990.0f, -990.0f, -990.0f),
    DirectX::XMFLOAT3(-990.0f, -990.0f, -990.0f),
    DirectX::XMFLOAT3(-990.0f, -990.0f, -990.0f)
};

const int CHEESE_VOXEL_COUNT = VOXEL_CHEESE_HEIGHT *
                                (VOXEL_CHEESE_DEPTH +
                                    (VOXEL_CHEESE_DEPTH / 2) * ((VOXEL_CHEESE_DEPTH / 2) + 1) / 2 +
                                    ((VOXEL_CHEESE_DEPTH + 1) / 2) * ((VOXEL_CHEESE_DEPTH + 1) / 2 - 1) / 2);

void OctreeNode::InsertVoxel(DirectX::XMFLOAT3 voxelPosition, int maxDepth, int currentDepth)
{
    // 최대 깊이에 도달하면 복셀 데이터를 현재 노드에 저장
    if (currentDepth >= maxDepth)
    {
        voxelData.emplace_back(DirectX::XMLoadFloat3(&voxelPosition));
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
        size_t og_size = voxelData.size();
        voxelData.erase(
            std::remove_if(voxelData.begin(), voxelData.end(), [&](const DirectX::XMVECTOR& voxel)
                {
                    return sphere.Contains(voxel);
                }),
            voxelData.end()
        );
        return og_size > voxelData.size();  // 복셀 삭제 여부 반환
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

bool OctreeNode::IntersectCheck(const DirectX::BoundingSphere& sphere, DirectX::BoundingBox& AABB) const
{
    std::vector<DirectX::BoundingBox> intersectedAABBs;

    // 충돌한 AABB를 탐색하는 재귀 함수 호출
    DiscoverAABB(sphere, intersectedAABBs);

    // 충돌한 AABB가 없으면 빈 AABB 반환
    if (intersectedAABBs.empty()) 
    {
        return false;
    }

    // 최소 점 및 최대 점 초기화
    DirectX::XMVECTOR minPoint = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 0.0f);
    DirectX::XMVECTOR maxPoint = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0.0f);

    // 충돌한 AABB 병합
    for (const auto& aabb : intersectedAABBs)
    {
        // AABB의 최소 및 최대 점 계산
        DirectX::XMVECTOR center = DirectX::XMLoadFloat3(&aabb.Center);
        DirectX::XMVECTOR extents = DirectX::XMLoadFloat3(&aabb.Extents);

        DirectX::XMVECTOR aabbMin = DirectX::XMVectorSubtract(center, extents);
        DirectX::XMVECTOR aabbMax = DirectX::XMVectorAdd(center, extents);

        // 최소 및 최대 점 갱신
        minPoint = DirectX::XMVectorMin(minPoint, aabbMin);
        maxPoint = DirectX::XMVectorMax(maxPoint, aabbMax);
    }

    // 병합된 AABB 계산
    DirectX::XMVECTOR mergedCenter = DirectX::XMVectorScale(DirectX::XMVectorAdd(minPoint, maxPoint), 0.5f);
    DirectX::XMVECTOR mergedExtents = DirectX::XMVectorScale(DirectX::XMVectorSubtract(maxPoint, minPoint), 0.5f);

    // 병합된 AABB를 반환
    DirectX::XMStoreFloat3(&AABB.Center, mergedCenter);
    DirectX::XMStoreFloat3(&AABB.Extents, mergedExtents);

    return true;
}

void OctreeNode::DiscoverAABB(const DirectX::BoundingSphere& sphere, std::vector<DirectX::BoundingBox>& result) const 
{
    // 현재 노드와 구의 충돌 확인
    if (false == boundingBox.Intersects(sphere)) 
    {
        return;
    }

    // 리프 노드인지 확인
    if (false == voxelData.empty()) 
    {
        // 리프 노드일 경우 충돌한 AABB 추가
        result.push_back(boundingBox);
        return;
    }

    // 자식 노드 탐색
    for (const auto& child : children) 
    {
        if (child) 
        {
            child->DiscoverAABB(sphere, result);
        }
    }
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