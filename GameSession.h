#pragma once
#include "Over_IO.h"

class Player;

class GameSession
{
public:
	std::unordered_map<int, Player> players_;
	//std::unordered_map<int, Npc> Npcs_;
	
	GameSession()
	{
		players_.clear();
		//Npcs_.clear();
	}
	~GameSession() {}



};

extern std::unordered_map <int, GameSession> g_sessions;