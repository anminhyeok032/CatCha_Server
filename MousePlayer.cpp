#include "MousePlayer.h"
#include "Player.h"

void MousePlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard ������Ʈ
	player->SetKeyState(action, is_key_pressed);

	// ���� ������Ʈ ��û
	player->RequestSessionUpdate();
}

bool MousePlayer::CalculatePhysics(Player* player, float deltaTime)
{
	bool is_moving = false;

	// ���� ȸ�� üũ
	if (player->prev_player_pitch_ != player->player_pitch_)
	{
		is_moving = true;
		player->prev_player_pitch_ = player->player_pitch_;
	}

	float time_remaining = (deltaTime < 1.0f) ? deltaTime : 1.0f;	// �ִ� 1�ʱ����� ���
	const int MAX_ITERATIONS = 100;									// ���� ���� ����
	int iterations = 0;												// ���� üũ

	while (time_remaining >= FIXED_TIME_STEP) 
	{
		player->delta_position_ = DirectX::XMFLOAT3();

		// �ӵ� ��� �� �̵� ���� üũ
		if (player->UpdateVelocity(FIXED_TIME_STEP))
		{
			is_moving = true;
		}

		// ��Ÿ ���� ó��
		player->ApplyDecelerationIfStop(FIXED_TIME_STEP);
		player->ApplyForces(FIXED_TIME_STEP);
		player->ApplyFriction(FIXED_TIME_STEP);

		// ��ġ ������Ʈ
		player->position_ = MathHelper::Add(player->position_, player->delta_position_);

		// ���� �ð� ���ܸ�ŭ ����
		time_remaining -= FIXED_TIME_STEP;

		iterations++;
	}

	return is_moving;
}
