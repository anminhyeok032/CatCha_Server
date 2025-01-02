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

	// 애니메이션 동기화 관련 변수들
	bool on_ground_ = false;
	Object_State obj_state_ = Object_State::STATE_IDLE;

	// 스킬 사용시 움직이지 않도록
	bool moveable_ = true;
	float stop_skill_time_ = 0.0f;

	DirectX::XMFLOAT3 direction_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 velocity_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 force_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 depth_delta_ = DirectX::XMFLOAT3();

	float player_pitch_;
	float prev_player_pitch_;

	DirectX::XMFLOAT3 delta_position_ = DirectX::XMFLOAT3();

	std::unique_ptr<CharacterState> state_;  // 현재 상태 객체

	// 플레이어 업데이트 여부
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

	// 상태 전환
	void SetState(std::unique_ptr<CharacterState> new_state);

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket();								// 첫 로그인 패킷 전송

	void ProcessPacket(char* packet) override;
	void SetCompletionKey(CompletionKey& key) override { comp_key_ = key; }

	SOCKET GetSocket() override { return socket_; }
	CompletionKey GetCompletionKey() override { return comp_key_; }

	DirectX::XMFLOAT3 GetVelocity() { return velocity_vector_; }
	DirectX::XMFLOAT3 GetForce() { return force_vector_; }

	// 키보드 입력 상태 관리 함수
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


	// 회전 업데이트
	void UpdateRotation(float degree);
	// Look, Up, Right 업데이트
	void UpdateLookUpRight();
	// 속도 업데이트
	bool UpdateVelocity(float time_step);
	// 정지 시 감속 적용
	void ApplyDecelerationIfStop(float time_step);
	// 힘 적용
	void ApplyForces(float time_step);
	// 마찰 적용
	void ApplyFriction(float time_step);
	// 중력 적용
	void ApplyGravity(float time_step);

	void Set_OBB(DirectX::BoundingOrientedBox obb);

	virtual void InputKey() {}
	// 움직임 변화 감지를 위한 bool return
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

