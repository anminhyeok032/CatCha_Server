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

    // 정해진 목표가 갈 수 없는 곳일때
    if(false == g_tile_map[target_x * TILE_X_CORR + target_z].walkable)
	{
		is_reached_ = false;
		return;
	}

    // 목표 노드가 유효하지 않을 경우 처리
    if (target_x < 0 || target_z < 0 || target_x >= TILE_MAP_WIDTH || target_z >= TILE_MAP_LENGTH) 
    {
        is_reached_ = false;
        return;
    }

    is_reached_ = false;
    
    // 컨테이너 초기화
    std::unordered_map<int, Node> open_map;
    std::unordered_set<int> closed_set;
    
    // 시작 노드 생성
    const int start_key = current_x_ * TILE_X_CORR + current_z_;
    open_map[start_key] = { 
        current_x_, current_z_,
        0.0f, static_cast<float>(ManhattanDistance(current_x_, current_z_, target_x, target_z)),
        nullptr 
    };

    // 방향 배열
    constexpr int directions[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };

    // A* 알고리즘
    while (!open_map.empty())
    {
        // OpenMap에서 FCost 값이 가장 작은 노드 (평가값이 가장 좋은) - 기존 노드는 closed_set에 있으므로 검사를 통해 제외
        auto curr_iter = std::min_element(open_map.begin(), open_map.end(), [&closed_set](const auto& a, const auto& b)
            { 
                if (closed_set.count(a.first)) return false;
                if (closed_set.count(b.first)) return true;
                return a.second.GetFCost() < b.second.GetFCost(); 
            });

        
        Node* curr = &curr_iter->second;
        const int curr_key = curr_iter->first;

        // clost set에 추가
        closed_set.insert(curr_key);

        // 목표에 도달했다면 경로 재구성
        if(curr->x == target_x && curr->z == target_z)
		{
			// 경로 재구성
			while (curr->parent != nullptr)
			{
				path_.push_back({ curr->x, curr->z });
				curr = curr->parent;
			}

			// 시작 노드 추가
			path_.push_back({ current_x_, current_z_ });

			// 경로 뒤집기
			std::reverse(path_.begin(), path_.end());
            is_reached_ = false;
			break;
		}

        // 인접 노드 탐색
        for (const auto& dir : directions)
        {
            int next_x = curr->x + dir[0];
            int next_z = curr->z + dir[1];
            const int next_key = next_x * TILE_X_CORR + next_z;

            // 타일맵이 못 지나가는 경우
            if (next_key < -1 || next_key >= g_tile_map.size() || false == g_tile_map[next_key].walkable)
            {
                continue;
            }

            // 클로즈 리스트에 있는 경우
            if (closed_set.find(next_key) != closed_set.end())
            {
                continue;
            }

            // 이동 비용 계산
            float new_gCost = curr->gCost + 1.0f;
        
            // Open List에 없는 노드거나 더 좋은 경로 발견 시 업데이트
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

    // 타일맵 원점 보정 값 - (0,0)이 -TILE_MAP_WIDTH / 2.0f 만큼 이동
    const float tile_map_offset_x = -TILE_MAP_WIDTH / 2.0f;
    const float tile_map_offset_z = -TILE_MAP_LENGTH / 2.0f;

    // 속도
    const float tile_per_speed = 100.0f * TILE_SIZE; // 칸/초

    // 남은 거리 계산 (현재 타일에서 다음 타일까지)
    float left_distance = tile_per_speed * deltaTime;

	// while 루프를 돌며 남은 거리가 0보다 크다면 계속해서 다음 노드로 이동
    while (left_distance > 0.0f && false == path_.empty())
    {
        // 현재 위치
        float current_x = static_cast<float>(current_x_) * TILE_SIZE + tile_map_offset_x;
        float current_z = static_cast<float>(current_z_) * TILE_SIZE + tile_map_offset_z;

        // 다음 타일 목표 위치
        std::pair<int, int> next_tile = path_.front();
        float target_x = static_cast<float>(next_tile.first) * TILE_SIZE + tile_map_offset_x;
        float target_z = static_cast<float>(next_tile.second) * TILE_SIZE + tile_map_offset_z;

        // 두 점 간의 거리 계산
        float dx = target_x - current_x;
        float dz = target_z - current_z;
        float dist_next_tile = std::sqrt(dx * dx + dz * dz);

        // 만약 다음 노드에 도착했다면 path_에서 다음 노드를 제거
        if (left_distance >= dist_next_tile)
        {
            // 다음 타일에 도달
            current_x_ = next_tile.first;
            current_z_ = next_tile.second;
            position_.x = target_x;
            position_.z = target_z;

            left_distance -= dist_next_tile;

            // 현재 경로에서 제거
            path_.erase(path_.begin());
        }
        // 시간 계산 값 이용해서 다음 노드로 이동해서 Position_ 업데이트
        else 
        {
            // 남은 거리만큼 이동
            float move_percent = left_distance / dist_next_tile;
            position_.x += dx * move_percent;
            position_.z += dz * move_percent;
            left_distance = 0.0f;
        }
    }

    SetBoundingSphere();

    // 경로가 비어 있다면 목표에 도달한 상태로 설정
    if (path_.empty()) 
    {
        //std::cout << "도착!" << std::endl;
        is_reached_ = true;
    }

    return true;
}
