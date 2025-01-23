#pragma once
#include "Octree.h"

constexpr int CHEESE_DETAIL_LEVEL = 2;

class VoxelPatternManager
{
public:
	std::array<OctreeNode, 5> voxel_patterns_;
    std::array<int, 5> random_seeds_;

    std::random_device rd;


public:
	VoxelPatternManager()
	{
        std::uniform_int_distribution<int> uid(1, 10);
		for (int i = 0; i < CHEESE_NUM; i++)
		{
            std::cout << "Create Voxel Pattern  - [" << i << "]" << std::endl;
            random_seeds_[i] = uid(rd);
            //std::cout << "Random Seed : " << random_seeds_[i] << std::endl;
			CrtVoxelCheeseOctree(voxel_patterns_[i], CHEESE_POS[i], CHEESE_SCALE, CHEESE_DETAIL_LEVEL, random_seeds_[i]);
		}
	}

    void VoxelPatternManager::CrtVoxelCheeseOctree(OctreeNode& root, DirectX::XMFLOAT3 position, float scale, UINT detail_level, int random_seed)
    {
        int count = 0;
        int m_random_value = 10;

        // �׻� ���� ����� ���� �ǻ� ���� ������
        std::mt19937 generator(random_seed);
        std::uniform_int_distribution<int> uid(1, m_random_value);

        DirectX::XMFLOAT3 pivot_position = position;
        position.y += scale / 2.0f;

        // ġ�� ��Ʈ�� �ʱ� ���� ���ϱ�
        root.halfSize = VOXEL_CHEESE_DEPTH * scale / 2.0f;              // ���� ������ �������� ��Ʈ�� ũ�� ����
        root.center = DirectX::XMFLOAT3(
            pivot_position.x,                                           // x�� �߽��� ���� x ��ǥ
            pivot_position.y + VOXEL_CHEESE_HEIGHT * scale / 2.0f,      // y�� �߽��� ������ �߰�
            pivot_position.z                                            // z�� �߽��� ���� z ��ǥ
        );
        root.boundingBox = DirectX::BoundingBox(root.center, { root.halfSize, root.halfSize, root.halfSize });

        for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i)
        {
            position.z = pivot_position.z - scale * (float)(VOXEL_CHEESE_DEPTH / 2);

            for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j)
            {
                position.x = pivot_position.x - scale * (float)(VOXEL_CHEESE_WIDTH / 2);

                for (int k = 0; k <= j / 2; ++k)
                {
                    if ((i == VOXEL_CHEESE_HEIGHT - 1 || j == VOXEL_CHEESE_DEPTH || k == 0 || k == j / 2) &&
                        !(uid(generator) % m_random_value)) 
                    {
                        position.x += scale;
                        continue;
                    }

                    // ��Ʈ���� ����
                    SubdivideVoxel(root, position, scale, detail_level);
                    position.x += scale;
                }

                position.z += scale;
            }

            position.y += scale;
        }
        //root.PrintNode();
    }

    // ���� ���� �Լ�
    void VoxelPatternManager::SubdivideVoxel(OctreeNode& node, DirectX::XMFLOAT3 position, float scale, UINT detail_level)
    {
        if (detail_level == 0)
        {
            // �� �̻� �������� �ʰ� ���� ����
            node.InsertVoxel(position, CHEESE_DETAIL_LEVEL + MAX_DEPTH, 0);
            return;
        }

        // ���� �������� �������� ���̰�, detail_level ��ŭ ���ο� �߽����� ����Ͽ� 8���� ���� ���� ����
        // 8 ^ detail_level
        float half = scale / 2.0f;
        float quarter = scale / 4.0f;

        for (int dx = -1; dx <= 1; dx += 2)
        {
            for (int dy = -1; dy <= 1; dy += 2)
            {
                for (int dz = -1; dz <= 1; dz += 2)
                {
                    // �� �������� ���ҵ� ������ �߽��� ���
                    DirectX::XMFLOAT3 new_position = {
                        position.x + quarter * dx,
                        position.y + quarter * dy,
                        position.z + quarter * dz
                    };

                    // ��������� ���� ���� ����
                    SubdivideVoxel(node, new_position, half, detail_level - 1);
                }
            }
        }
    }
};

extern VoxelPatternManager g_voxel_pattern_manager;