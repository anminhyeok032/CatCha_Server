#pragma once
#include "Over_IO.h"
#include "Character.h"
#include "Octree.h"
#include "VoxelPattenManager.h"

// 해당 세션 상태
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

	std::unordered_map<int, std::unique_ptr<Player>> players_;			// [key] = player_index <주의* CHARACTER_NUMBER 아님> / [value] = player
	std::unordered_map<int, std::unique_ptr<AIPlayer>> ai_players_;		// [key] = player_index <주의* CHARACTER_NUMBER 아님> / [value] = ai_player

	SESSION_STATE session_state_ = SESSION_STATE::SESSION_WAIT;

	std::mutex mt_session_state_;

	int session_num_ = -1;

	// ElapsedTime 계산을 위한 변수
	uint64_t lastupdatetime_ = 0;
	uint64_t last_game_time_ = 0;

	// 해당 세션의 남은 시간
	short remaining_time_ = 0;

	// 업데이트 호출 횟수
	int update_count_ = 0;

	// 고양이의 공격 OBB
	DirectX::BoundingOrientedBox cat_attack_obb_{ 
		DirectX::XMFLOAT3{0, -9999.0f, 0},
		DirectX::XMFLOAT3{0, 0, 0}, 
		DirectX::XMFLOAT4{1, 1, 1, 0} 
	};
	// 공격한 방향
	DirectX::XMFLOAT3 cat_attack_direction_{ 0, 0, 0 };
	// 현재 공격중인지
	bool cat_attack_ = false;
	// 공격당한 쥐 판별
	std::unordered_map<int, bool> cat_attacked_player_;			// [key] = CHARACTER_NUMBER / [value] = is_attacked

	// 치즈 옥트리
	std::vector<OctreeNode> cheese_octree_;

public:
	GameSession()
	{
		players_.clear();
		cheese_octree_.clear();
		lastupdatetime_ = GetServerTime();
		remaining_time_ = 300;	// 5분
		cat_attack_obb_.Center = DirectX::XMFLOAT3(0, -9999.0f, 0);
		for (int i = 0; i < CHEESE_NUM; i++)
		{
			cheese_octree_.emplace_back(g_voxel_pattern_manager.voxel_patterns_[i]);
			//CrtVoxelCheeseOctree(cheese_octree_[i], CHEESE_POS[i], CHEESE_SCALE, 0);
			//cheese_octree_[i] = g_voxel_pattern_manager.voxel_patterns_[i].DeepCopy();
		}
	}

	// 임시 세션용 생성자
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
	void BroadcastGameStart();															// 게임 시작 Init && 시작 패킷 브로드캐스팅
	void BroadcastPosition(int player);													// 같은 세션 내 플레이어 위치 브로드캐스팅
	void BroadcastSync();																// 특정 주기마다 서버와 클라이언트 위치 및 쿼터니언 동기화 브로드캐스팅
	void BroadcastChangeCharacter(int player_num, int CHARACTER_NUM);					// player_index, 바꿀 Character_number 캐릭터 변경 브로드캐스팅
	void BroadcastAddCharacter(int player_num, int recv_index);							// 접속한 캐릭터 추가 브로드캐스팅
	void BroadcastRemoveVoxelSphere(int cheese_num, const DirectX::XMFLOAT3& center);	// 삭제할 치즈와 boundingSphere 중심점 브로드캐스팅
	void BroadcastAIPostion(int num);													// AI 캐릭터 위치 브로드캐스팅
	void BroadcastTime();																// 게임 시간 브로드 캐스팅

	//===========================
	// IO 스레드로 Send 요청(PQCS)
	//===========================
	void SendAIUpdate(int move_AIs);
	void SendPlayerUpdate(int move_players);

	void SetCharacter(int room_num, int client_index, bool is_cat);

	int GetMouseNum();
	void CheckAttackedMice();
	void DeleteCheeseVoxel(const DirectX::XMFLOAT3& center);
	void InitializeSessionAI();
	bool RebornToAI(int player_num);
};

extern std::unordered_map <int, GameSession> g_sessions;