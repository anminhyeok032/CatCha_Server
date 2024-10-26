#pragma once
#include "Player.h"

class CatPlayer : public Player
{
public:
	CatPlayer()	{
		std::cout << "고양이 객체 생성" << std::endl;
		id_ = NUM_CAT;
	}

	~CatPlayer() {}

	// KeyInput
	void InputKey() override;

	// 움직임 변화 감지를 위한 bool return
	bool UpdatePosition(float deltaTime) override;
};