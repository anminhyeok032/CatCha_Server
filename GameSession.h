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

	// ElapsedTime ����� ���� ����
	//std::atomic<std::chrono::steady_clock::time_point> lastUpdateTime;
	uint64_t lastupdatetime_;
	uint64_t last_game_time_;

	// �ش� ������ ���� �ð�
	int remaining_time_;

	GameSession()
	{
		characters_.clear();
		lastupdatetime_ = GetServerTime();
		remaining_time_ = 300;	// 5��
	}
	~GameSession() {}

	int CheckCharacterNum() { return characters_.size(); }

	void Update();
	uint64_t GetServerTime();
	void SendPlayerUpdate();
	void SendTimeUpdate();
};

extern std::unordered_map <int, GameSession> g_sessions;