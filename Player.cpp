#include "Player.h"

void Player::DoReceive()
{
	DWORD recv_flag = 0;
	memset(&recv_over_.over_, 0, sizeof(recv_over_.over_));
	recv_over_.wsabuf_.buf = recv_over_.send_buf_ + prev_packet_.size();
	recv_over_.wsabuf_.len = BUF_SIZE - prev_packet_.size();
	WSARecv(socket_, &recv_over_.wsabuf_, 1, 0, &recv_flag,
		&recv_over_.over_, 0);
}

void Player::DoSend(void* packet)
{
	Over_IO* send_over = new Over_IO {reinterpret_cast<char*>(prev_packet_.data())};
	WSASend(socket_, &send_over->wsabuf_, 1, 0, 0, &send_over->over_, 0);
}

void Player::ProcessPacket(char* packet)
{
	switch (packet[1])
	{
		// 로그인 패킷 처리
	case CS_LOGIN:
		break;
	case CS_MOVE:
		break;

	}
}
