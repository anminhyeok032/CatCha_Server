#pragma once
#include "GameSession.h"

class Character
{
public:
	// ĳ���� ����
	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3();
	int max_hp_, curr_hp_;

	// ���� pitch ��ȭ�� �ѷ�
	float total_pitch_;

	DirectX::XMFLOAT4 rotation_quat_ = { 0, 0, 0, 1 };						// �ʱ� ���ʹϾ� (���� ���ʹϾ�)
	DirectX::XMFLOAT4X4 rotation_matrix_ = MathHelper::Identity_4x4();		// ȸ�� ���
	
	DirectX::XMFLOAT3 m_look = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };

	bool dirty_ = false;  // ȸ�� ���°� ����Ǿ����� Ȯ��

	// ĳ���� ��ȣ
	int id_;		// 0~3 mouse, 4~7 AI, 8 cat
	bool is_cat_;	// true : Cat, false : Mouse

	// ���� �� �÷��̾� ��ȣ
	CompletionKey comp_key_;

	// packet ������
	std::vector<char> prev_packet_;

	// ��Ŷ ó��
	Over_IO recv_over_;
	SOCKET socket_;

	// Ŭ���̾�Ʈ �ּ�
	sockaddr_in client_addr_;

	// �г��� 

	// KeyInput
	std::unordered_map<Action, bool> keyboard_input_;
	uint8_t key_;

	// physics
	bool is_jumping_;





public:
	// Player �ʱ�ȭ�� ���� ���� �Լ�
	virtual void SetSocket(SOCKET socket) {}
	virtual void DoReceive() {}
	virtual bool ProcessPacket(char* packet) { return true; }
	virtual void DoSend(void* packet) {}
	virtual void SetCompletionKey(CompletionKey& key) {}

	virtual SOCKET GetSocket() = 0;
	virtual CompletionKey GetCompletionKey() = 0;

	virtual void InputKey() {}

	virtual bool UpdatePosition(float deltaTime) { return false; }
};
