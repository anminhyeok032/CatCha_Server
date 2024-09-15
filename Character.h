#pragma once
#include "GameSession.h"


class Character : public GameSession
{
public:
	float x_, y_, z_;
	// 동기화를 위한 스냅샷 좌표
	float prev_x_, prev_y_, prev_z_;
	int max_hp_, curr_hp_;
	
	// 닉네임 

	// physics
	float velocity_;
	bool is_jumping_;

	// 세션 및 플레이어 번호
	CompletionKey comp_key_;

	sockaddr_in client_addr_;

	// packet 재조립
	std::vector<char> prev_packet_;

public:
	// Player 초기화를 위한 가상 함수
	virtual void SetSocket(SOCKET socket) {}
	virtual void DoReceive() {}
	virtual void ProcessPacket(char* packet) {}
	virtual void DoSend(void* packet) {}
	virtual void SetAddr() {}
	void SetCompletionKey(CompletionKey& key) {	comp_key_ = key; }

	virtual bool UpdatePosition(float deltaTime) { return false; }
};
