#pragma once
#include "Player.h"

class CatPlayer : public Player
{
public:
	CatPlayer()	{
		std::cout << "����� ��ü ����" << std::endl;
		id_ = NUM_CAT;
	}

	~CatPlayer() {}

	// KeyInput
	void InputKey() override;

	// ������ ��ȭ ������ ���� bool return
	bool UpdatePosition(float deltaTime) override;
};