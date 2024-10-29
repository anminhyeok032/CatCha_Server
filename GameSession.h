#pragma once
#include "Over_IO.h"
#include "Character.h"


enum SESSION_STATE {
	SESSION_WAIT,
	SESSION_FULL
};

class Character;
class Player;
class CatPlayer;
class MousePlayer;

class GameSession
{
public:

	std::unordered_map<int, std::unique_ptr<Player>> players_;			// [key] = player_index / [value] = player

	SESSION_STATE state_;

	std::mutex mt_session_state_;

	int session_num_;

	// ElapsedTime 계산을 위한 변수
	uint64_t lastupdatetime_;
	uint64_t last_game_time_;

	// 해당 세션의 남은 시간
	int remaining_time_;

	// udp 처리를 위한 값
	SOCKET udp_socket_;
	Concurrency::concurrent_unordered_map<int, Packet> packetBuffer_; // 시퀀스 번호를 키로 하는 패킷 버퍼
	int expectedSequenceNumber_ = 0;               // 다음에 처리할 패킷의 시퀀스 번호

	// 업데이트 호출 횟수
	int update_count_ = 0;

	// Update Dirty Flag
	std::atomic<bool> dirty_{ false };  

	GameSession()
	{
		players_.clear();
		lastupdatetime_ = GetServerTime();
		remaining_time_ = 300;	// 5분
	}
	~GameSession() {}

	size_t CheckCharacterNum() const { return players_.size(); }

	bool Update();
	uint64_t GetServerTime();
	void SendPlayerUpdate(int move_players);
	void SendTimeUpdate();

	void InitUDPSocket();
	void BroadcastPosition(int player);
	void BroadcastSync();
	void BroadcastChangeCharacter(int player_num, int CHARACTER_NUM);
	void BroadcastAddCharacter(int player_num, int recv_index);

	void SetCharacter(int room_num, int client_index, bool is_cat);

	int GetMouseNum();

	// 업데이트 중복 체크
	void MarkDirty();
	bool IsDirty() const;
	void ClearDirty();
};

extern std::unordered_map <int, GameSession> g_sessions;