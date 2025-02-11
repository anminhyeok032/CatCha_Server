#pragma once
#include "Character.h"

class CharacterState;

class Player : public Character
{
public:
	// �÷��̾� ���� ����
	PLAYER_STATE player_server_state_ = PLAYER_STATE::PS_FREE;
	std::mutex mt_player_server_state_;

	// �÷��̾� ����
	char name[NAME_SIZE];
	char password[PASSWORD_SIZE];
	int	curr_hp_ = 0;	// ���� ü��
	int reborn_ai_character_id_ = -1;	// ��Ȱ��ų AI ĳ���� ��ȣ

	// physics
	float speed_ = 0.0f;				// ���� �ӵ�
	float max_speed_ = 150.f;			// �ִ� �ӵ�
	float acceleration_ = 50.0f;		// ����
	float deceleration_ = 1000.0f;		// ����

	float jump_power_ = 400.0f;

	// �ִϸ��̼� ����ȭ ���� ������
	bool on_ground_ = false;
	Object_State obj_state_ = Object_State::STATE_IDLE;
	// ������ ������Ʈ�� �ʿ��Ҷ�
	bool force_move_update_ = false;

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

	std::unique_ptr<CharacterState> character_state_;  // ���� ���� ��ü

	// �÷��̾� ������Ʈ ����
	std::atomic<bool> needs_update_{ false };

	bool request_send_escape_ = false;
	bool request_send_dead_ = false;
	bool request_send_reborn_ = false;

	// bite�� ����� ���� ����
	DirectX::XMFLOAT3 bite_center_ = DirectX::XMFLOAT3();

	Player()
	{
		character_id_ = NUM_GHOST;
		curr_hp_ = 100;
		key_ = 0;
		socket_ = INVALID_SOCKET;
	}

	~Player() 
	{
		std::cout << "Player Destructor" << std::endl;
	}

	void SetSocket(SOCKET socket) override { socket_ = socket; }
	void SetID(int id) override { character_id_ = id; }

	// ���� ��ȯ
	void SetState(std::unique_ptr<CharacterState> new_state);

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket(bool result);								// ù �α��� ��Ŷ ����
	void SendMyPlayerNumber();											// �ڽ��� �÷��̾� ��ȣ ����	
	void SendRandomCheeseSeedPacket();									// ġ�� �õ� ����

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

	DirectX::XMFLOAT3 GetLook()		const { return look_; }
	DirectX::XMFLOAT3 GetUp()		const { return up_; }
	DirectX::XMFLOAT3 GetRight()	const { return right_; }

	void ResetPlayer();
};
