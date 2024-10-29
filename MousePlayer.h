#pragma once
#include "CharacterState.h"

class MousePlayer : public CharacterState
{
public:
	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// 움직임 변화 감지를 위한 bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;
};