#pragma once
#include "Character.h"

class Player : public Character
{
public:
	Over_IO recv_over_;
	int id_;
	
	SOCKET socket_;


	// KeyInput
	std::unordered_map<Action, bool> keyboard_input_;
	uint8_t key_;
	XMFLOAT3 direction_vector_ = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 velocity_vector_ = XMFLOAT3(0.f, 0.f, 0.f);
	float max_speed_ = 1000.f;

	Player()
	{
		id_ = 0;
		x_ = 0.0f;
		y_ = 0.0f;
		z_ = 0.0f;
		prev_x_ = 0.0f;
		prev_y_ = 0.0f;
		prev_z_ = 0.0f;
		max_hp_ = 100;
		curr_hp_ = 100;
		velocity_ = 0.0f;
		is_jumping_ = false;
	}
	~Player() {}

	void SetSocket(SOCKET socket) override { socket_ = socket; }

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket();
	void ProcessPacket(char* packet) override;

	// 움직임 변화 감지를 위한 bool return
	bool UpdatePosition(float deltaTime) override;

	// UDP를 위한 소켓 주소 설정
	void SetAddr() override;

	void InputKey();
};

