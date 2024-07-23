#pragma once
#include "Over_IO.h"

class Character;

class GameSession
{
public:
	std::unordered_map<int, std::unique_ptr<Character>> players_;

	
	GameSession()
	{
		players_.clear();
		//Npcs_.clear();
	}
	~GameSession() {}



};

extern std::unordered_map <int, GameSession> g_sessions;