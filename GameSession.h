#pragma once
#include "Over_IO.h"

class Character;

enum SESSION_STATE {
	SESSION_WAIT,
	SESSION_FULL
};

class GameSession
{
public:
	std::unordered_map<int, std::unique_ptr<Character>> characters_;

	SESSION_STATE state_;
	std::mutex mt_session_state_;

	// ElapsedTime 계산을 위한 변수
	//std::atomic<std::chrono::steady_clock::time_point> lastUpdateTime;
	std::chrono::steady_clock::time_point lastUpdateTime;
	
	GameSession()
	{
		characters_.clear();
	}
	~GameSession() {}

	int CheckCharacterNum() { return characters_.size(); }

};

extern std::unordered_map <int, GameSession> g_sessions;