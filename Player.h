#pragma once
#include "Character.h"


class Player : public Character
{
public:
	// physics
	float speed_ = 0.0f;
	float max_speed_ = 200.f;
	float acceleration_ = 100.0f;
	float deceleration_ = 1000.0f;
	XMFLOAT3 direction_vector_ = DirectX::XMFLOAT3();
	XMFLOAT3 velocity_vector_ = DirectX::XMFLOAT3();
	XMFLOAT3 force_vector_ = DirectX::XMFLOAT3();

	float player_pitch_;
	float prev_player_pitch_;

	DirectX::XMFLOAT3 delta_position_ = DirectX::XMFLOAT3();

	Player()
	{
		id_ = 0;
		max_hp_ = 100;
		curr_hp_ = 100;
		is_jumping_ = false;
		key_ = 0;
		socket_ = INVALID_SOCKET;
	}

	~Player() 
	{
		std::cout << "Player Destructor" << std::endl;
	}

	void SetSocket(SOCKET socket) override { socket_ = socket; }
	void SetAddr();
	void SetID(int id) { id_ = id; }

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket();								// ù �α��� ��Ŷ ����

	bool ProcessPacket(char* packet) override;
	void SetCompletionKey(CompletionKey& key) override { comp_key_ = key; }

	SOCKET GetSocket() override { return socket_; }
	CompletionKey GetCompletionKey() override { return comp_key_; }

	DirectX::XMFLOAT3 GetVelocity() { return velocity_vector_; }
	DirectX::XMFLOAT3 GetForce() { return force_vector_; }



	// ȸ�� ������Ʈ
	void UpdateRotation(float yaw);
	// Look, Up, Right ������Ʈ
	void UpdateLookUpRight();
	// �ӵ� ������Ʈ
	bool UpdateVelocity(float time_step);
	// ���� �� ���� ����
	void ApplyDecelerationIfStop(float time_step);
	// �� ����
	void ApplyForces(float time_step);
	// ���� ����
	void ApplyFriction(float time_step);
	// �߷� ����
	void ApplyGravity(float time_step);

	virtual void InputKey() {}
	// ������ ��ȭ ������ ���� bool return
	virtual bool UpdatePosition(float deltaTime) { return false; }

	void Move_Forward();
	void Move_Back();
	void Move_Left();
	void Move_Right();

	DirectX::XMFLOAT3 GetLook()		const { return m_look; }
	DirectX::XMFLOAT3 GetUp()		const { return m_up; }
	DirectX::XMFLOAT3 GetRight()	const { return m_right; }

};

