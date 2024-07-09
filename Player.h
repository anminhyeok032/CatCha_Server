#pragma once
#include "GameSession.h"

class Player : public GameSession
{
public:
	Over_IO recv_over_;
	int id_;
	float x_, y_, z_;
	// 동기화를 위한 스냅샷
	float prev_x_, prev_y_, prev_z_;
	int max_hp_, curr_hp_;

	// packet 재조립
	std::vector<char> prev_packet_;

	// physics
	float velocity_;
	bool isJumping_;

	Player()
	{
		id_ = -1;
		x_ = 0.0f;
		y_ = 0.0f;
		z_ = 0.0f;
		prev_x_ = 0.0f;
		prev_y_ = 0.0f;
		prev_z_ = 0.0f;
		max_hp_ = 100;
		curr_hp_ = 100;
		velocity_ = 0.0f;
		isJumping_ = false;
	}
	~Player() {}

	void DoReceive();
	void DoSend();

	void ProcessPacket(char* packet);
};

