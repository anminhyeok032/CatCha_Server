#pragma once
#include "GameSession.h"


class Character : public GameSession
{
public:
	float x_, y_, z_;
	// ����ȭ�� ���� ������ ��ǥ
	float prev_x_, prev_y_, prev_z_;
	int max_hp_, curr_hp_;

	// physics
	float velocity_;
	bool isJumping_;
};
