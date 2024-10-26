#include "CatPlayer.h"

void CatPlayer::InputKey()
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action key = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard 업데이트
	keyboard_input_[key] = is_key_pressed;


	XMFLOAT3 input_vector = XMFLOAT3(0.f, 0.f, 0.f);

	// Process keyboard input
	for (const auto& entry : keyboard_input_)
	{
		Action key_char = entry.first;
		bool is_pressed = entry.second;

		if (is_pressed)
		{
			switch (key_char)
			{
				// Movement
			case Action::MOVE_FORWARD:
				Move_Forward();
				break;
			case Action::MOVE_BACK:
				Move_Back();
				break;
			case Action::MOVE_LEFT:
				Move_Left();
				break;
			case Action::MOVE_RIGHT:
				Move_Right();
				break;
				// TODO : 클라이언트 점프 구현 후 추가 구현
				//case ' ':
				//	// Jump
				//	if (false == is_jumping_)
				//	{
				//		input_vector = Vector3::Add(input_vector, { 0, 1, 0 });
				//		is_jumping_ = true;
				//	}
				//	break;
			default:
				std::cout << "Invalid key input" << std::endl;
				break;
			}
		}

	}

	// 키 입력에 따른 이동 업데이트
	commandQueue.push(comp_key_.session_id);
}

bool CatPlayer::UpdatePosition(float deltaTime)
{
	bool is_moving = false;

	if (prev_player_pitch_ != player_pitch_)
	{
		is_moving = true;
		prev_player_pitch_ = player_pitch_;
	}

	float time_remaining = 0.0f;

	// 남은 시간을 누적
	time_remaining += deltaTime;

	while (time_remaining >= FIXED_TIME_STEP)
	{
		delta_position_ = DirectX::XMFLOAT3();

		// 속도 계산 및 검사
		if (UpdateVelocity(FIXED_TIME_STEP))
		{
			is_moving = true;
		}

		// 대기 상태일 때 감속 적용 및 검사
		ApplyDecelerationIfStop(FIXED_TIME_STEP);

		// 힘 적용
		ApplyForces(FIXED_TIME_STEP);

		// 마찰 적용
		ApplyFriction(FIXED_TIME_STEP);

		// 중력 적용
		//ApplyGravity(FIXED_TIME_STEP);

		// 위치 업데이트
		position_ = MathHelper::Add(position_, delta_position_);

		time_remaining -= FIXED_TIME_STEP;  // 고정 시간 스텝만큼 누적된 시간을 감소시킴
	}


	return is_moving;
}
