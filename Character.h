#pragma once
#include "GameSession.h"


class Character : public GameSession
{
public:
	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3();
	int max_hp_, curr_hp_;

	float player_pitch_;
	float total_pitch_;
	float prev_player_pitch_;

	DirectX::XMFLOAT4 rotation_quat_ = { 0, 0, 0, 1 };  // 초기 쿼터니언 (단위 쿼터니언)
	DirectX::XMFLOAT4X4 rotation_matrix_ = MathHelper::Identity_4x4();               // 회전 행렬
	
	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	bool dirty_ = false;  // 회전 상태가 변경되었는지 확인

	// 닉네임 

	// physics
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
