#pragma once
#include "Character.h"

class CharacterState;

class Player : public Character
{
public:
	// physics
	float speed_ = 0.0f;
	float max_speed_ = 200.f;
	float acceleration_ = 100.0f;
	float deceleration_ = 1000.0f;

	float jump_power_ = 500.0f;

	// �ִϸ��̼� ����ȭ ���� ������
	bool on_ground_ = false;
	Object_State obj_state_ = Object_State::STATE_IDLE;

	// ��ų ���� �������� �ʵ���
	bool moveable_ = true;
	float stop_skill_time_ = 0.0f;

	DirectX::XMFLOAT3 direction_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 velocity_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 force_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 depth_delta_ = DirectX::XMFLOAT3();

	float player_pitch_;
	float prev_player_pitch_;

	DirectX::XMFLOAT3 delta_position_ = DirectX::XMFLOAT3();

	std::unique_ptr<CharacterState> state_;  // ���� ���� ��ü

	// �÷��̾� ������Ʈ ����
	std::atomic<bool> needs_update_{ false };

	Player()
	{
		id_ = NUM_GHOST;
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

	// ���� ��ȯ
	void SetState(std::unique_ptr<CharacterState> new_state);

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket();								// ù �α��� ��Ŷ ����

	void ProcessPacket(char* packet) override;
	void SetCompletionKey(CompletionKey& key) override { comp_key_ = key; }

	SOCKET GetSocket() override { return socket_; }
	CompletionKey GetCompletionKey() override { return comp_key_; }

	DirectX::XMFLOAT3 GetVelocity() { return velocity_vector_; }
	DirectX::XMFLOAT3 GetForce() { return force_vector_; }

	// Ű���� �Է� ���� ���� �Լ�
	void SetKeyState(Action action, bool is_pressed) 
	{
		keyboard_input_[action] = is_pressed;
	}

	void RequestUpdate() 
	{
		if (false == needs_update_.load())
		{
			TIMER_EVENT ev{ std::chrono::system_clock::now(), comp_key_.session_id };
			commandQueue.push(ev);
			needs_update_.store(true);
		}
	}


	// ȸ�� ������Ʈ
	void UpdateRotation(float degree);
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

	void Set_OBB(DirectX::BoundingOrientedBox obb);

	virtual void InputKey() {}
	// ������ ��ȭ ������ ���� bool return
	bool UpdatePosition(float deltaTime);


	void MoveForward();
	void MoveBack();
	void MoveLeft();
	void MoveRight();
	void Jump();

	DirectX::XMFLOAT3 GetLook()		const { return look_; }
	DirectX::XMFLOAT3 GetUp()		const { return up_; }
	DirectX::XMFLOAT3 GetRight()	const { return right_; }

};

