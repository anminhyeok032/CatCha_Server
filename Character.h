#pragma once
#include "GameSession.h"


class Character : public GameSession
{
public:
	float x_, y_, z_;
	// µ¿±âÈ­¸¦ À§ÇÑ ½º³À¼¦ ÁÂÇ¥
	float prev_x_, prev_y_, prev_z_;
	int max_hp_, curr_hp_;

	// physics
	float velocity_;
	bool isJumping_;
};
