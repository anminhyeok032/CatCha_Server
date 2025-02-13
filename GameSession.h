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
class AIPlayer;
class CatPlayer;
class MousePlayer;
class OctreeNode;
class VoxelPatternManager;

class GameSession
{
public:

	std::unordered_map<int, std::unique_ptr<Player>> players_;			// [key] = player_index <����* CHARACTER_NUMBER �ƴ�> / [value] = player
	std::unordered_map<int, std::unique_ptr<AIPlayer>> ai_players_;		// [key] = player_index <����* CHARACTER_NUMBER �ƴ�> / [value] = ai_player

	SESSION_STATE session_state_ = SESSION_STATE::SESSION_WAIT;
	std::mutex mt_session_state_;

	std::unordered_map<int, bool> alive_mouse_;							// [key] = ����ִ� ���� ���� (player_index) / [value] = Ż�� ����, ���� ���� - (������ �����ȴ�)
	std::vector<int> escape_mouse_;										// Ż���� ���� ��ȣ(player_index)
	std::vector<int> broken_cheese_num_;								// �ٸ��� ġ�� ��ȣ(cheese_num)

	bool is_game_start_ = false;										// �ش� ���� ���� ���� ����
	bool is_door_open_ = false;											// ���� ���ӿ��� ���� ���� ����
	std::atomic<bool> is_reset_ai_{ false };							// ���� ������ ����ƴ��� �ٸ� ������� ���Ͽ� ���� �������� ����� �����忡�� ���� ������ �����ֱ� ���� bool
	std::atomic<bool> is_reset_timer_{ false };							// ���� ������ ����ƴ��� �ٸ� ������� ���Ͽ� ���� �������� ����� �����忡�� ���� ������ �����ֱ� ���� bool
	std::atomic<bool> game_over_{ false };								// ���� ������ ����ƴ��� �ٸ� ������� ���Ͽ� ���� �������� ����� �����忡�� ���� ������ �����ֱ� ���� bool
	
	int session_num_ = -1;

	// ElapsedTime ����� ���� ����
	uint64_t lastupdatetime_ = 0;
	uint64_t last_game_time_ = 0;

	// �ش� ������ ���� �ð�
	short remaining_time_ = 0;

	// ������Ʈ ȣ�� Ƚ��
	int update_count_ = 0;

	// ������� ���� OBB
	DirectX::BoundingOrientedBox cat_attack_obb_{ 
		DirectX::XMFLOAT3{0, -9999.0f, 0},
		DirectX::XMFLOAT3{0, 0, 0}, 
		DirectX::XMFLOAT4{1, 1, 1, 0} 
	};
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
	void UpdateAI();

	void StartSessionUpdate()
	{
		std::cout << "Create GameSession - " << session_num_ << std::endl;
		TIMER_EVENT ev{ std::chrono::system_clock::now(), session_num_ };
		commandQueue.push(ev);
	}

	uint64_t GetServerTime();

	//===========================
	// Broadcasting
	//===========================
	void BroadcastGameStart();															// ���� ���� Init && ���� ��Ŷ ��ε�ĳ����
	void BroadcastPosition(int player);													// ���� ���� �� �÷��̾� ��ġ ��ε�ĳ����
	void BroadcastSync();																// Ư�� �ֱ⸶�� ������ Ŭ���̾�Ʈ ��ġ �� ���ʹϾ� ����ȭ ��ε�ĳ����
	void BroadcastChangeCharacter(int player_num, int CHARACTER_NUM);					// player_index, �ٲ� Character_number ĳ���� ���� ��ε�ĳ����
	void BroadcastAddCharacter(int player_num, int recv_index);							// ������ ĳ���� �߰� ��ε�ĳ����
	void BroadcastRemoveVoxelSphere(
		int cheese_num, const DirectX::XMFLOAT3& center, bool is_removed_all);			// ������ ġ��� boundingSphere �߽��� ��ε�ĳ����
	void BroadcastAIPostion(int num);													// AI ĳ���� ��ġ ��ε�ĳ����
	void BroadcastTime();																// ���� �ð� ��ε� ĳ����
	void BroadcastDoorOpen();															// ġ�� ���� ������ �� ���� ��ε� ĳ����
	void BroadcastCatWin();																// ����� �¸� ��Ŷ ��ε� ĳ����
	void BroadcastMouseWin();															// �� �¸� ��Ŷ ��ε� ĳ����
	void BroadcastEscape();																// Ż���� �� ��ε� ĳ����
	void BroadcastReborn();																// ȯ���ϴ� �㿡�� ��Ŷ ����(�ش��ϴ� �㿡�Ը�)
	void BroadcastDead();																// ȯ�� �Ұ� ���� �㿡�� ��Ŷ ����(�ش��ϴ� �㿡�Ը�)


	//===========================
	// IO ������� Send ��û(PQCS)
	//===========================
	void RequestSendAIUpdate(int move_AIs);												// AI ��ġ ������Ʈ IO�� ��û
	void RequestSendPlayerUpdate(int move_players);										// �÷��̾� ��ġ ������Ʈ IO�� ��û
	void RequestSendGameEvent(GAME_EVENT ge);											// ���� �̺�Ʈ ������Ʈ IO�� ��û


	void SetCharacter(int room_num, int client_index, bool is_cat);

	int GetMouseNum();
	void CheckAttackedMice();
	void DeleteCheeseVoxel(const DirectX::XMFLOAT3& center);							// ġ�� ���� �� ���� ������ �� ����
	void InitializeSessionAI();
	bool RebornToAI(int player_num);

	bool CheckGameOver();																// ���� ���� Ȯ��
	void CheckResult();																	// ���� ��� Ȯ��
	void MovePlayerToWaitngSession();													// ���� ���� ��, ��� �������� �ű�� �Լ�
};

extern std::unordered_map <int, GameSession> g_sessions;