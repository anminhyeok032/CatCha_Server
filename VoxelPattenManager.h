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

        // 항상 같은 결과를 내는 의사 난수 생성기
        std::mt19937 generator(random_seed);
        std::uniform_int_distribution<int> uid(1, m_random_value);

        DirectX::XMFLOAT3 pivot_position = position;
        position.y += scale / 2.0f;

        // 치즈 옥트리 초기 기준 정하기
        root.halfSize = VOXEL_CHEESE_DEPTH * scale / 2.0f;              // 가장 긴축을 기준으로 옥트리 크기 설정
        root.center = DirectX::XMFLOAT3(
            pivot_position.x,                                           // x축 중심은 시작 x 좌표
            pivot_position.y + VOXEL_CHEESE_HEIGHT * scale / 2.0f,      // y축 중심은 높이의 중간
            pivot_position.z                                            // z축 중심은 시작 z 좌표
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

                    // 옥트리에 삽입
                    SubdivideVoxel(root, position, scale, detail_level);
                    position.x += scale;
                }

                position.z += scale;
            }

            position.y += scale;
        }
        //root.PrintNode();
    }

    // 복셀 분할 함수
    void VoxelPatternManager::SubdivideVoxel(OctreeNode& node, DirectX::XMFLOAT3 position, float scale, UINT detail_level)
    {
        if (detail_level == 0)
        {
            // 더 이상 분할하지 않고 복셀 삽입
            node.InsertVoxel(position, CHEESE_DETAIL_LEVEL + MAX_DEPTH, 0);
            return;
        }

        // 현재 스케일을 절반으로 줄이고, detail_level 만큼 새로운 중심점을 계산하여 8개의 하위 복셀 생성
        // 8 ^ detail_level
        float half = scale / 2.0f;
        float quarter = scale / 4.0f;

        for (int dx = -1; dx <= 1; dx += 2)
        {
            for (int dy = -1; dy <= 1; dy += 2)
            {
                for (int dz = -1; dz <= 1; dz += 2)
                {
                    // 각 방향으로 분할된 복셀의 중심점 계산
                    DirectX::XMFLOAT3 new_position = {
                        position.x + quarter * dx,
                        position.y + quarter * dy,
                        position.z + quarter * dz
                    };

                    // 재귀적으로 하위 복셀 분할
                    SubdivideVoxel(node, new_position, half, detail_level - 1);
                }
            }
        }
    }
};

extern VoxelPatternManager g_voxel_pattern_manager;