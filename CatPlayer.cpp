#include "CatPlayer.h"

void CatPlayer::InputKey()
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action key = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard ������Ʈ
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
				// TODO : Ŭ���̾�Ʈ ���� ���� �� �߰� ����
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

	// Ű �Է¿� ���� �̵� ������Ʈ
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

	// ���� �ð��� ����
	time_remaining += deltaTime;

	while (time_remaining >= FIXED_TIME_STEP)
	{
		delta_position_ = DirectX::XMFLOAT3();

		// �ӵ� ��� �� �˻�
		if (UpdateVelocity(FIXED_TIME_STEP))
		{
			is_moving = true;
		}

		// ��� ������ �� ���� ���� �� �˻�
		ApplyDecelerationIfStop(FIXED_TIME_STEP);

		// �� ����
		ApplyForces(FIXED_TIME_STEP);

		// ���� ����
		ApplyFriction(FIXED_TIME_STEP);

		// �߷� ����
		//ApplyGravity(FIXED_TIME_STEP);

		// ��ġ ������Ʈ
		position_ = MathHelper::Add(position_, delta_position_);

		time_remaining -= FIXED_TIME_STEP;  // ���� �ð� ���ܸ�ŭ ������ �ð��� ���ҽ�Ŵ
	}


	return is_moving;
}
