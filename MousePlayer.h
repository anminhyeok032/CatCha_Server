#pragma once
#include "CharacterState.h"

class MousePlayer : public CharacterState
{
public:
	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// ������ ��ȭ ������ ���� bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;
};