#pragma once
#include "Character.h"

class Player : public Character
{
public:
	Over_IO recv_over_;
	int id_;
	
	SOCKET socket_;

	// KeyInput
	std::unordered_map<char, bool> keyboard_input_;
	uint8_t Direction;
	XMFLOAT3 direction_vector_ = XMFLOAT3(0.f, 0.f, 0.f);

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

	void SetSocket(SOCKET socket) override { socket_ = socket; }

	void DoReceive() override;
	void DoSend(void* packet);

	void ProcessPacket(char* packet) override;

	void InputKey();
};

