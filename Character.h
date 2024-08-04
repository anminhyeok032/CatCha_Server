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

	// packet ������
	std::vector<char> prev_packet_;

public:
	// Player �ʱ�ȭ�� ���� ���� �Լ�
	virtual void SetSocket(SOCKET socket) {}
	virtual void DoReceive() {}
	virtual void ProcessPacket(char* packet) {}
	virtual void DoSend(void* packet) {}
};
