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
    // �ִ� ���̿� �����ϸ� ���� �����͸� ���� ��忡 ����
    if (currentDepth >= maxDepth)
    {
        voxelData.emplace_back(DirectX::XMLoadFloat3(&voxelPosition));
        return;
    }

    // ������ ��ǥ�� �������� ��� �ڽ� ��忡 ���� ���� 
    int childIndex = 0;
    if (voxelPosition.x > center.x) childIndex |= 1;    // x�� �������� ������ ����.
    if (voxelPosition.y > center.y) childIndex |= 2;    // y�� �������� ���� ����.
    if (voxelPosition.z > center.z) childIndex |= 4;    // z�� �������� ���� ����.

    // �ش� �ڽ� ��尡 ��Ȱ�� ���¶�� ���ο� ��带 ����.
    if (!children[childIndex])
    {
        // ���ο� �ڽ� ����� �߽���
        DirectX::XMFLOAT3 childCenter = {
            center.x + ((childIndex & 1) ? halfSize / 2.0f : -halfSize / 2.0f),
            center.y + ((childIndex & 2) ? halfSize / 2.0f : -halfSize / 2.0f),
            center.z + ((childIndex & 4) ? halfSize / 2.0f : -halfSize / 2.0f)
        };

        // ���ο� �ڽ� ��带 �����ϰ� ���� ����� �ڽ� �迭�� ����
        children[childIndex] = std::make_unique<OctreeNode>(childCenter, halfSize / 2.0f);

        // ���ο� �ڽ� ����� AABB �ּ�/�ִ� ��ǥ ���
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

        // �ش� �ڽ� ��带 Ȱ��ȭ
        activeFlags[childIndex] = true;
    }

    // ��������� �ڽ� ��忡 ���� �����͸� ����
    children[childIndex]->InsertVoxel(voxelPosition, maxDepth, currentDepth + 1);
}

bool OctreeNode::RemoveVoxel(const DirectX::BoundingSphere& sphere)
{
    // ���� ����� AABB�� ���� ���� ���� Ȯ��
    if (false == boundingBox.Intersects(sphere))
    {
        return false;
    }

    // Leaf ����� ���
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
        return og_size > voxelData.size();  // ���� ���� ���� ��ȯ
    }

    // ���� ���� ����� Ž��
    bool is_child_changed = false;
    for (auto& child : children)
    {
        if (child && child->RemoveVoxel(sphere))
        {
            is_child_changed = true;
        }
    }

    // ��� ���� ��尡 ��� �ִٸ� ����
    if (true == is_child_changed)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (children[i] && children[i]->IsEmpty())
            {
                children[i].reset();        // �ڽ� ��� ����
                activeFlags[i] = false;     // Ȱ�� �÷��� ��Ȱ��ȭ
            }
        }
    }

    return is_child_changed;
}

bool OctreeNode::IntersectCheck(const DirectX::BoundingSphere& sphere, DirectX::BoundingBox& AABB) const
{
    std::vector<DirectX::BoundingBox> intersectedAABBs;

    // �浹�� AABB�� Ž���ϴ� ��� �Լ� ȣ��
    DiscoverAABB(sphere, intersectedAABBs);

    // �浹�� AABB�� ������ �� AABB ��ȯ
    if (intersectedAABBs.empty()) 
    {
        return false;
    }

    // �ּ� �� �� �ִ� �� �ʱ�ȭ
    DirectX::XMVECTOR minPoint = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 0.0f);
    DirectX::XMVECTOR maxPoint = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0.0f);

    // �浹�� AABB ����
    for (const auto& aabb : intersectedAABBs)
    {
        // AABB�� �ּ� �� �ִ� �� ���
        DirectX::XMVECTOR center = DirectX::XMLoadFloat3(&aabb.Center);
        DirectX::XMVECTOR extents = DirectX::XMLoadFloat3(&aabb.Extents);

        DirectX::XMVECTOR aabbMin = DirectX::XMVectorSubtract(center, extents);
        DirectX::XMVECTOR aabbMax = DirectX::XMVectorAdd(center, extents);

        // �ּ� �� �ִ� �� ����
        minPoint = DirectX::XMVectorMin(minPoint, aabbMin);
        maxPoint = DirectX::XMVectorMax(maxPoint, aabbMax);
    }

    // ���յ� AABB ���
    DirectX::XMVECTOR mergedCenter = DirectX::XMVectorScale(DirectX::XMVectorAdd(minPoint, maxPoint), 0.5f);
    DirectX::XMVECTOR mergedExtents = DirectX::XMVectorScale(DirectX::XMVectorSubtract(maxPoint, minPoint), 0.5f);

    // ���յ� AABB�� ��ȯ
    DirectX::XMStoreFloat3(&AABB.Center, mergedCenter);
    DirectX::XMStoreFloat3(&AABB.Extents, mergedExtents);

    return true;
}

void OctreeNode::DiscoverAABB(const DirectX::BoundingSphere& sphere, std::vector<DirectX::BoundingBox>& result) const 
{
    // ���� ���� ���� �浹 Ȯ��
    if (false == boundingBox.Intersects(sphere)) 
    {
        return;
    }

    // ���� ������� Ȯ��
    if (false == voxelData.empty()) 
    {
        // ���� ����� ��� �浹�� AABB �߰�
        result.push_back(boundingBox);
        return;
    }

    // �ڽ� ��� Ž��
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
    // ���� �����Ͱ� ������ ��� ���� ����
    if (false == voxelData.empty())
    {
        return false;
    }

    // �ڽ� ��尡 ������ ��� ���� ����
    for (int i = 0; i < 8; ++i)
    {
        if (children[i])
        {
            return false;
        }
    }

    // ���� �����Ϳ� �ڽ� ��尡 ��� �����
    return true;
}

void OctreeNode::PrintNode(int depth) const
{
    // Ȱ��ȭ�� �ڽ� ���
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

    // Ȱ��ȭ�� �ڽ� ��带 ��������� ���
    for (int i = 0; i < 8; ++i)
    {
        if (children[i])
        {
            children[i]->PrintNode(depth + 1);
        }
    }
}