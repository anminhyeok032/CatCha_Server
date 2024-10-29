#include "MousePlayer.h"
#include "Player.h"

void MousePlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard 업데이트
	player->SetKeyState(action, is_key_pressed);

	// 세션 업데이트 요청
	player->RequestSessionUpdate();
}

bool MousePlayer::CalculatePhysics(Player* player, float deltaTime)
{
	bool is_moving = false;

	// 각도 회전 체크
	if (player->prev_player_pitch_ != player->player_pitch_)
	{
		is_moving = true;
		player->prev_player_pitch_ = player->player_pitch_;
	}

	float time_remaining = (deltaTime < 1.0f) ? deltaTime : 1.0f;	// 최대 1초까지만 계산
	const int MAX_ITERATIONS = 100;									// 무한 루프 방지
	int iterations = 0;												// 루프 체크

	while (time_remaining >= FIXED_TIME_STEP) 
	{
		player->delta_position_ = DirectX::XMFLOAT3();

		// 속도 계산 및 이동 여부 체크
		if (player->UpdateVelocity(FIXED_TIME_STEP))
		{
			is_moving = true;
		}

		// 기타 물리 처리
		player->ApplyDecelerationIfStop(FIXED_TIME_STEP);
		player->ApplyForces(FIXED_TIME_STEP);
		player->ApplyFriction(FIXED_TIME_STEP);

		// 위치 업데이트
		player->position_ = MathHelper::Add(player->position_, player->delta_position_);

		// 고정 시간 스텝만큼 감소
		time_remaining -= FIXED_TIME_STEP;

		iterations++;
	}

	return is_moving;
}
