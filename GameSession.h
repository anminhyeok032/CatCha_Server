#pragma once
#include "Over_IO.h"
#include "Character.h"
#include "Octree.h"
#include "VoxelPattenManager.h"

// �ش� ���� ����
enum SESSION_STATE {
	SESSION_WAIT,
	SESSION_HAS_CAT,
	SESSION_FULL
};

class Character;
class Player;
class CatPlayer;
class MousePlayer;
class OctreeNode;
class VoxelPatternManager;

class GameSession
{
public:

	std::unordered_map<int, std::unique_ptr<Player>> players_;			// [key] = player_index <����* CHARACTER_NUMBER �ƴ�> / [value] = player

	SESSION_STATE session_state_ = SESSION_STATE::SESSION_WAIT;

	std::mutex mt_session_state_;

	int session_num_ = -1;

	// ElapsedTime ����� ���� ����
	uint64_t lastupdatetime_ = 0;
	uint64_t last_game_time_ = 0;

	// �ش� ������ ���� �ð�
	int remaining_time_ = 0;

	// ������Ʈ ȣ�� Ƚ��
	int update_count_ = 0;

	// ������� ���� OBB
	DirectX::BoundingOrientedBox cat_attack_obb_{ DirectX::XMFLOAT3{0, -9999.0f, 0}, DirectX::XMFLOAT3{0, 0, 0}, DirectX::XMFLOAT4{1, 1, 1, 0} };
	// ������ ����
	DirectX::XMFLOAT3 cat_attack_direction_{ 0, 0, 0 };
	// ���� ����������
	bool cat_attack_ = false;
	// ���ݴ��� �� �Ǻ�
	std::unordered_map<int, bool> cat_attacked_player_;			// [key] = CHARACTER_NUMBER / [value] = is_attacked

	// ġ�� ��Ʈ��
	std::vector<OctreeNode> cheese_octree_;

public:
	GameSession()
	{
		players_.clear();
		cheese_octree_.clear();
		lastupdatetime_ = GetServerTime();
		remaining_time_ = 300;	// 5��
		cat_attack_obb_.Center = DirectX::XMFLOAT3(0, -9999.0f, 0);
		for (int i = 0; i < CHEESE_NUM; i++)
		{
			cheese_octree_.emplace_back(g_voxel_pattern_manager.voxel_patterns_[i]);
			//CrtVoxelCheeseOctree(cheese_octree_[i], CHEESE_POS[i], CHEESE_SCALE, 0);
			//cheese_octree_[i] = g_voxel_pattern_manager.voxel_patterns_[i].DeepCopy();
		}
	}

	// �ӽ� ���ǿ� ������
	GameSession(int session_num) : session_num_(session_num)
	{
		players_.clear();
		cheese_octree_.clear();
	}

	~GameSession() {}

	size_t CheckCharacterNum() const { return players_.size(); }

	void Update();
	void StartSessionUpdate()
	{
		std::cout << "Create GameSession - " << session_num_ << std::endl;
		TIMER_EVENT ev{ std::chrono::system_clock::now(), session_num_ };
		commandQueue.push(ev);
	}
	uint64_t GetServerTime();
	void SendPlayerUpdate(int move_players);
	void SendTimeUpdate();

	void BroadcastPosition(int player);
	void BroadcastSync();
	void BroadcastChangeCharacter(int player_num, int CHARACTER_NUM);
	void BroadcastAddCharacter(int player_num, int recv_index);
	void BroadcastRemoveVoxelSphere(int cheese_num, const DirectX::XMFLOAT3& center);

	void SetCharacter(int room_num, int client_index, bool is_cat);

	int GetMouseNum();
	void CheckAttackedMice();
	void DeleteCheeseVoxel(const DirectX::XMFLOAT3& center);
};

extern std::unordered_map <int, GameSession> g_sessions;