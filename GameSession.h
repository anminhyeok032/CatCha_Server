#pragma once
#include "Over_IO.h"
#include "Character.h"


enum SESSION_STATE {
	SESSION_WAIT,
	SESSION_FULL
};

class Character;
class Player;
class CatPlayer;
class MousePlayer;

class GameSession
{
public:

	std::unordered_map<int, std::unique_ptr<Player>> players_;			// [key] = player_index / [value] = player

	SESSION_STATE state_;

	std::mutex mt_session_state_;

	int session_num_;

	// ElapsedTime ����� ���� ����
	uint64_t lastupdatetime_;
	uint64_t last_game_time_;

	// �ش� ������ ���� �ð�
	int remaining_time_;

	// udp ó���� ���� ��
	SOCKET udp_socket_;
	Concurrency::concurrent_unordered_map<int, Packet> packetBuffer_; // ������ ��ȣ�� Ű�� �ϴ� ��Ŷ ����
	int expectedSequenceNumber_ = 0;               // ������ ó���� ��Ŷ�� ������ ��ȣ

	// ������Ʈ ȣ�� Ƚ��
	int update_count_ = 0;

	// Update Dirty Flag
	std::atomic<bool> dirty_{ false };  

	GameSession()
	{
		players_.clear();
		lastupdatetime_ = GetServerTime();
		remaining_time_ = 300;	// 5��
	}
	~GameSession() {}

	size_t CheckCharacterNum() const { return players_.size(); }

	bool Update();
	uint64_t GetServerTime();
	void SendPlayerUpdate(int move_players);
	void SendTimeUpdate();

	void InitUDPSocket();
	void BroadcastPosition(int player);
	void BroadcastSync();
	void BroadcastChangeCharacter(int player_num, int CHARACTER_NUM);
	void BroadcastAddCharacter(int player_num, int recv_index);

	void SetCharacter(int room_num, int client_index, bool is_cat);

	int GetMouseNum();

	// ������Ʈ �ߺ� üũ
	void MarkDirty();
	bool IsDirty() const;
	void ClearDirty();
};

extern std::unordered_map <int, GameSession> g_sessions;