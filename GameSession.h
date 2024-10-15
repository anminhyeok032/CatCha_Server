#pragma once
#include "Over_IO.h"

enum SESSION_STATE {
	SESSION_WAIT,
	SESSION_FULL
};

class Character;

class GameSession
{
public:
	std::unordered_map<int, std::unique_ptr<Character>> characters_;

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

	GameSession()
	{
		characters_.clear();
		lastupdatetime_ = GetServerTime();
		remaining_time_ = 300;	// 5분
	}
	~GameSession() {}

	int CheckCharacterNum() const { return characters_.size(); }

	bool Update();
	uint64_t GetServerTime();
	void SendPlayerUpdate(int move_players);
	void SendTimeUpdate();

	void InitUDPSocket();
	void BroadcastPosition(int player);
	void BroadcastPosition();
};

extern std::unordered_map <int, GameSession> g_sessions;