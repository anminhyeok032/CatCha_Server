#include "AIPlayer.h"
#include "Player.h"
#include "CharacterState.h"
#include <unordered_set>

std::vector<Tile> g_tile_map;
std::vector<int> g_tile_map_walkable_only;
std::random_device dre;

void AIPlayer::SetBoundingSphere()
{
	bounding_sphere_.Center = position_;
    bounding_sphere_.Center.y += 5.104148f / 2.0f;
}

int AIPlayer::ManhattanDistance(int x1, int y1, int x2, int y2) 
{
    return abs(x1 - x2) + abs(y1 - y2);
}

void AIPlayer::FindPath(int target_x, int target_z)
{
    path_.clear();

    // ������ ��ǥ�� �� �� ���� ���϶�
    if(false == g_tile_map[target_x * TILE_X_CORR + target_z].walkable)
	{
		is_reached_ = false;
		return;
	}

    // ��ǥ ��尡 ��ȿ���� ���� ��� ó��
    if (target_x < 0 || target_z < 0 || target_x >= TILE_MAP_WIDTH || target_z >= TILE_MAP_LENGTH) 
    {
        is_reached_ = false;
        return;
    }

    is_reached_ = false;
    
    // �����̳� �ʱ�ȭ
    std::unordered_map<int, Node> open_map;
    std::unordered_set<int> closed_set;
    
    // ���� ��� ����
    const int start_key = current_x_ * TILE_X_CORR + current_z_;
    open_map[start_key] = { 
        current_x_, current_z_,
        0.0f, static_cast<float>(ManhattanDistance(current_x_, current_z_, target_x, target_z)),
        nullptr 
    };

    // ���� �迭
    constexpr int directions[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };

    // A* �˰���
    while (!open_map.empty())
    {
        // OpenMap���� FCost ���� ���� ���� ��� (�򰡰��� ���� ����) - ���� ���� closed_set�� �����Ƿ� �˻縦 ���� ����
        auto curr_iter = std::min_element(open_map.begin(), open_map.end(), [&closed_set](const auto& a, const auto& b)
            { 
                if (closed_set.count(a.first)) return false;
                if (closed_set.count(b.first)) return true;
                return a.second.GetFCost() < b.second.GetFCost(); 
            });

        
        Node* curr = &curr_iter->second;
        const int curr_key = curr_iter->first;

        // clost set�� �߰�
        closed_set.insert(curr_key);

        // ��ǥ�� �����ߴٸ� ��� �籸��
        if(curr->x == target_x && curr->z == target_z)
		{
			// ��� �籸��
			while (curr->parent != nullptr)
			{
				path_.push_back({ curr->x, curr->z });
				curr = curr->parent;
			}

			// ���� ��� �߰�
			path_.push_back({ current_x_, current_z_ });

			// ��� ������
			std::reverse(path_.begin(), path_.end());
            is_reached_ = false;
			break;
		}

        // ���� ��� Ž��
        for (const auto& dir : directions)
        {
            int next_x = curr->x + dir[0];
            int next_z = curr->z + dir[1];
            const int next_key = next_x * TILE_X_CORR + next_z;

            // Ÿ�ϸ��� �� �������� ���
            if (next_key < -1 || next_key >= g_tile_map.size() || false == g_tile_map[next_key].walkable)
            {
                continue;
            }

            // Ŭ���� ����Ʈ�� �ִ� ���
            if (closed_set.find(next_key) != closed_set.end())
            {
                continue;
            }

            // �̵� ��� ���
            float new_gCost = curr->gCost + 1.0f;
        
            // Open List�� ���� ���ų� �� ���� ��� �߰� �� ������Ʈ
            if (open_map.find(next_key) == open_map.end() || new_gCost < open_map[next_key].gCost)
			{
				open_map[next_key] = { 
					next_x, next_z,
					new_gCost, static_cast<float>(ManhattanDistance(next_x, next_z, target_x, target_z)),
					curr
				};
			}


        }
    }
}

bool AIPlayer::UpdatePosition(float deltaTime)
{
	if (path_.empty() || true == is_reached_)
	{
        static std::uniform_int_distribution<int> uid{ 0, static_cast<int>(g_tile_map_walkable_only.size() - 1) };
        int rand_tile = uid(dre);

        int tile = g_tile_map_walkable_only[rand_tile];

        int target_x = tile / TILE_X_CORR;
        int target_z = tile % TILE_X_CORR;

        FindPath(target_x, target_z);
        return false;
	}

    // Ÿ�ϸ� ���� ���� �� - (0,0)�� -TILE_MAP_WIDTH / 2.0f ��ŭ �̵�
    const float tile_map_offset_x = -TILE_MAP_WIDTH / 2.0f;
    const float tile_map_offset_z = -TILE_MAP_LENGTH / 2.0f;

    // �ӵ�
    const float tile_per_speed = 100.0f * TILE_SIZE; // ĭ/��

    // ���� �Ÿ� ��� (���� Ÿ�Ͽ��� ���� Ÿ�ϱ���)
    float left_distance = tile_per_speed * deltaTime;

	// while ������ ���� ���� �Ÿ��� 0���� ũ�ٸ� ����ؼ� ���� ���� �̵�
    while (left_distance > 0.0f && false == path_.empty())
    {
        // ���� ��ġ
        float current_x = static_cast<float>(current_x_) * TILE_SIZE + tile_map_offset_x;
        float current_z = static_cast<float>(current_z_) * TILE_SIZE + tile_map_offset_z;

        // ���� Ÿ�� ��ǥ ��ġ
        std::pair<int, int> next_tile = path_.front();
        float target_x = static_cast<float>(next_tile.first) * TILE_SIZE + tile_map_offset_x;
        float target_z = static_cast<float>(next_tile.second) * TILE_SIZE + tile_map_offset_z;

        // �� �� ���� �Ÿ� ���
        float dx = target_x - current_x;
        float dz = target_z - current_z;
        float dist_next_tile = std::sqrt(dx * dx + dz * dz);

        // ���� ���� ��忡 �����ߴٸ� path_���� ���� ��带 ����
        if (left_distance >= dist_next_tile)
        {
            // ���� Ÿ�Ͽ� ����
            current_x_ = next_tile.first;
            current_z_ = next_tile.second;
            position_.x = target_x;
            position_.z = target_z;

            left_distance -= dist_next_tile;

            // ���� ��ο��� ����
            path_.erase(path_.begin());
        }
        // �ð� ��� �� �̿��ؼ� ���� ���� �̵��ؼ� Position_ ������Ʈ
        else 
        {
            // ���� �Ÿ���ŭ �̵�
            float move_percent = left_distance / dist_next_tile;
            position_.x += dx * move_percent;
            position_.z += dz * move_percent;
            left_distance = 0.0f;
        }
    }

    SetBoundingSphere();

    // ��ΰ� ��� �ִٸ� ��ǥ�� ������ ���·� ����
    if (path_.empty()) 
    {
        //std::cout << "����!" << std::endl;
        is_reached_ = true;
    }

    return true;
}
