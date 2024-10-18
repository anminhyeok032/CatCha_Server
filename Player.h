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

	// physics
	float speed_ = 0.0f;
	float max_speed_ = 200.f;
	float acceleration_ = 100.0f;
	float deceleration_ = 200.0f;
	XMFLOAT3 direction_vector_ = DirectX::XMFLOAT3();
	XMFLOAT3 velocity_vector_ = DirectX::XMFLOAT3();
	XMFLOAT3 force_vector_ = DirectX::XMFLOAT3();



	DirectX::XMFLOAT3 delta_position_ = DirectX::XMFLOAT3();

	Player()
	{
		id_ = 0;
		max_hp_ = 100;
		curr_hp_ = 100;
		is_jumping_ = false;
	}
	~Player() {}

	void SetSocket(SOCKET socket) override { socket_ = socket; }

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket();
	void ProcessPacket(char* packet) override;

	DirectX::XMFLOAT3 GetVelocity() { return velocity_vector_; }
	DirectX::XMFLOAT3 GetForce() { return force_vector_; }

	// 움직임 변화 감지를 위한 bool return
	bool UpdatePosition(float deltaTime) override;
	void UpdateRotation(float yaw);
	void UpdateLookUpRight();

	bool UpdateVelocity(float time_step);

	void ApplyDecelerationIfStop(float time_step);

	void ApplyForces(float time_step);

	void ApplyFriction(float time_step);

	void ApplyGravity(float time_step);

	// UDP를 위한 소켓 주소 설정
	void SetAddr() override;

	void InputKey();

	void Move_Forward();
	void Move_Back();
	void Move_Left();
	void Move_Right();

	DirectX::XMFLOAT3 GetLook()		const { return m_look; }
	DirectX::XMFLOAT3 GetUp()		const { return m_up; }
	DirectX::XMFLOAT3 GetRight()	const { return m_right; }

};

