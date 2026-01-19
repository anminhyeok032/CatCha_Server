#pragma once
#include "Character.h"

class CharacterState;

class Player : public Character
{
public:
	// 플레이어 서버 상태
	PLAYER_STATE player_server_state_ = PLAYER_STATE::PS_FREE;
	std::mutex mt_player_server_state_;

	// 플레이어 정보
	char name[NAME_SIZE];
	char password[PASSWORD_SIZE];
	int	curr_hp_ = 100;	// 현재 체력
	int reborn_ai_character_id_ = -1;	// 부활시킬 AI 캐릭터 번호

	// physics
	float speed_ = 0.0f;				// 현재 속도
	const float max_speed_ = 150.f;			// 최대 속도
	const float acceleration_ = 50.0f;		// 가속
	const float deceleration_ = 1000.0f;		// 감속

	float jump_power_ = 400.0f;

	// 받은 pitch 변화값 총량
	float total_pitch_ = 0;
	// 고양이 차징 점프에 사용하는 yaw 변화값 총량
	float total_yaw_ = 0;

	// 애니메이션 동기화 관련 변수들
	bool on_ground_ = false;
	Object_State obj_state_ = Object_State::STATE_IDLE;
	// 움직임 업데이트가 필요할때
	bool force_move_update_ = false;

	// 스킬 사용시 움직이지 않도록
	bool moveable_ = true;
	float stop_skill_time_ = 0.0f;
	float jump_charging_time_ = 0.0f;

	DirectX::XMFLOAT3 direction_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 velocity_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 force_vector_ = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 depth_delta_ = DirectX::XMFLOAT3();

	float player_pitch_ = 0.0f;
	float prev_player_pitch_ = 0.0f;

	DirectX::XMFLOAT3 delta_position_ = DirectX::XMFLOAT3();

	std::unique_ptr<CharacterState> character_state_;  // 현재 상태 객체

	// 플레이어 업데이트 여부
	std::atomic<bool> needs_update_{ false };
	std::atomic<bool> disconnect_{ false };

	bool request_send_escape_ = false;
	bool request_send_dead_ = false;
	bool request_send_reborn_ = false;

	// 지연 보정을 위한 시간 변수
	std::chrono::system_clock::time_point last_packet_arrival_time_;
	const float EXPECTED_PACKET_INTERVAL = UPDATE_PERIOD; // (클라이언트 전송 주기에 맞춰 수정)
	const float MAX_LATENCY_COMPENSATION = 0.2f;   // 최대 0.2초까지만 예측 보정 (순간이동 방지)

	// 지연 보정값을 누적할 아토믹 변수 (락 프리)
	std::atomic<float> pending_latency_x_{ 0.0f };
	std::atomic<float> pending_latency_z_{ 0.0f };

	// bite시 생기는 구의 중점
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
	}

	void SetSocket(SOCKET socket) override { socket_ = socket; }
	void SetID(int id) override { character_id_ = id; }

	// 상태 전환
	void SetState(std::unique_ptr<CharacterState> new_state);

	void DoReceive() override;
	void DoSend(void* packet) override;
	void SendLoginInfoPacket(bool result);								// 첫 로그인 패킷 전송
	void SendMyPlayerNumber();											// 자신의 플레이어 번호 전송	
	void SendRandomCheeseSeedPacket();									// 치즈 시드 전송

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
			needs_update_.store(true);
		}
	}


	// 회전 업데이트
	void UpdatePitch(float degree);	
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


	void Set_OBB(DirectX::BoundingOrientedBox obb);

	virtual void InputKey() {}
	// 움직임 변화 감지를 위한 bool return
	bool UpdatePosition(float deltaTime);

	DirectX::XMFLOAT3 GetInputDirection(uint8_t key_input);

	// 지연 보정 함수
	void CompensateLatency(float latency, DirectX::XMFLOAT3 direction);


	void MoveForward();
	void MoveBack();
	void MoveLeft();
	void MoveRight();

	DirectX::XMFLOAT3 GetLook()		const { return look_; }
	DirectX::XMFLOAT3 GetUp()		const { return up_; }
	DirectX::XMFLOAT3 GetRight()	const { return right_; }

	void ResetPlayer();

	void AtomicAddFloat(std::atomic<float>& target, float value) {
		float current = target.load();
		while (!target.compare_exchange_weak(current, current + value));
	}
};
