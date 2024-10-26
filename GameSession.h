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

class GameSession
{
public:

	std::unordered_map<int, std::unique_ptr<Player>> players_;
	std::mutex	mt_packet_buffer_;

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
	void BroadcastPosition();
	void BroadcastChangeCharacter(int player_num, int CHARACTER_NUM);

	void SetCharacter(int room_num, int client_id, bool is_cat);
};

extern std::unordered_map <int, GameSession> g_sessions;