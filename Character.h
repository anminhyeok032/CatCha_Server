#pragma once
#include "GameSession.h"


class Character : public GameSession
{
public:
	float x_, y_, z_;
	// 동기화를 위한 스냅샷 좌표
	float prev_x_, prev_y_, prev_z_;
	int max_hp_, curr_hp_;

	// physics
	float velocity_;
	bool isJumping_;

	// packet 재조립
	std::vector<char> prev_packet_;

public:
	// Player 초기화를 위한 가상 함수
	virtual void SetSocket(SOCKET socket) {}
	virtual void DoReceive() {}
	virtual void ProcessPacket(char* packet) {}
	virtual void DoSend(void* packet) {}
};
