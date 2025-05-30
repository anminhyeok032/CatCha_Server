#pragma once
#include "GameSession.h"

class Character
{
public:
	// 캐릭터 정보
	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3();


	DirectX::XMFLOAT4 rotation_quat_ = { 0, 0, 0, 1 };						// 초기 쿼터니언 (단위 쿼터니언)
	DirectX::XMFLOAT4X4 rotation_matrix_ = MathHelper::Identity_4x4();		// 회전 행렬
	
	DirectX::XMFLOAT3 look_ = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 up_ = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 right_ = { 1.0f, 0.0f, 0.0f };

	bool dirty_ = false;  // 회전 상태가 변경되었는지 확인

	// 캐릭터 번호
	int character_id_ = -1;					// [CHARACTER NUM] 0~3 mouse, 4~7 AI, 8 cat

	// 세션 및 플레이어 번호
	CompletionKey comp_key_{};		// 세션 번호, 서버 플레이어 번호

	// packet 재조립
	std::vector<char> prev_packet_;

	// 패킷 처리
	Over_IO recv_over_;
	SOCKET socket_ = INVALID_SOCKET;


	// 닉네임 

	// KeyInput
	std::unordered_map<Action, bool> keyboard_input_;
	uint8_t key_ = 0;


public:
	// Player 초기화를 위한 가상 함수
	virtual void SetSocket(SOCKET socket) {}
	virtual void DoReceive() {}
	virtual void ProcessPacket(char* packet) {}
	virtual void DoSend(void* packet) {}
	virtual void SetCompletionKey(CompletionKey& key) {}

	virtual SOCKET GetSocket() = 0;
	virtual CompletionKey GetCompletionKey() = 0;

	virtual void SetID(int id) { }

};
