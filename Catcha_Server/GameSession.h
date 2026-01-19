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

	std::unordered_map<int, bool> alive_mouse_;							// [key] = 살아있는 생쥐 여부 (player_index) / [value] = 탈출 여부, 생존 여부 - (죽으면 삭제된다)
	std::vector<int> escape_mouse_;										// 탈출한 생쥐 번호(player_index)
	std::vector<int> broken_cheese_num_;								// 다먹은 치즈 번호(cheese_num)

	bool is_game_start_ = false;										// 해당 세션 게임 시작 여부
	bool is_door_open_ = false;											// 현재 게임에서 문이 열린 상태
	std::atomic<bool> is_reset_ai_{ false };							// 종료 로직이 실행됐는지 다른 스레드와 비교하여 가장 마지막에 종료된 스레드에게 종료 로직을 시켜주기 위한 bool
	std::atomic<bool> is_reset_timer_{ false };							// 종료 로직이 실행됐는지 다른 스레드와 비교하여 가장 마지막에 종료된 스레드에게 종료 로직을 시켜주기 위한 bool
	std::atomic<bool> game_over_{ false };								// 종료 로직이 실행됐는지 다른 스레드와 비교하여 가장 마지막에 종료된 스레드에게 종료 로직을 시켜주기 위한 bool
	
	int session_num_ = -1;

	// ElapsedTime 계산을 위한 변수
	uint64_t lastupdatetime_ = 0;
	uint64_t lastupdatetime_ai_ = 0;
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

	// 자투리 시간을 저장할 변수
	double accumulator_ = 0.0;

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
	void BroadcastRemoveVoxelSphere(
		int cheese_num, const DirectX::XMFLOAT3& center, bool is_removed_all);			// 삭제할 치즈와 boundingSphere 중심점 브로드캐스팅
	void BroadcastAIPostion(int num);													// AI 캐릭터 위치 브로드캐스팅
	void BroadcastTime();																// 게임 시간 브로드 캐스팅
	void BroadcastDoorOpen();															// 치즈 전부 삭제시 문 열기 브로드 캐스팅
	void BroadcastCatWin();																// 고양이 승리 패킷 브로드 캐스팅
	void BroadcastMouseWin();															// 쥐 승리 패킷 브로드 캐스팅
	void BroadcastEscape();																// 탈출한 쥐 브로드 캐스팅
	void BroadcastReborn();																// 환생하는 쥐에게 패킷 전송(해당하는 쥐에게만)
	void BroadcastDead();																// 환샌 불가 죽은 쥐에게 패킷 전송(해당하는 쥐에게만)


	//===========================
	// IO 스레드로 Send 요청(PQCS)
	//===========================
	void RequestSendAIUpdate(int move_AIs);												// AI 위치 업데이트 IO에 요청
	void RequestSendPlayerUpdate(int move_players);										// 플레이어 위치 업데이트 IO에 요청
	void RequestSendGameEvent(GAME_EVENT ge);											// 게임 이벤트 업데이트 IO에 요청


	void SetCharacter(int room_num, int client_index, bool is_cat);

	int GetMouseNum();
	void CheckAttackedMice();															// 공격 OBB와 Player 쥐 충돌체크
	bool CheckAttackedAI();																// 공격 OBB와 AI 쥐 충돌 체크
	void DeleteCheeseVoxel(const DirectX::XMFLOAT3& center);							// 치즈 삭제 및 전부 삭제시 문 열기
	void InitializeSessionAI();
	bool RebornToAI(int player_num);

	bool CheckGameOver();																// 게임 종료 확인
	void CheckResult();																	// 게임 결과 확인
	void MovePlayerToWaitngSession();													// 게임 종료 후, 대기 세션으로 옮기는 함수
	void DisconnectPlayer(int num);														// 플레이어 연결 끊김
};

extern std::unordered_map <int, GameSession> g_sessions;