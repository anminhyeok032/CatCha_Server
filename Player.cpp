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
	{
		// TODO : 이동 처리 먼저 하고 클라이언트끼리 동기화 하는 코드 작성
		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		Direction = p->keyinput;
		InputKey();
		break;
	}
	case CS_TIME:
	{
		CS_TIME_PACKET* p = reinterpret_cast<CS_TIME_PACKET*>(packet);
		remaining_time_ = p->time;
		std::cout << "게임 시간 : " << remaining_time_ << std::endl;
		break;
	}
	}
}

void Player::InputKey()
{
	bool is_key_pressed = Direction & 0x01;
	uint8_t key_stroke = Direction >> 1;
	char key = static_cast<char>(key_stroke);

	//std::cout << "key input : " << key << " = " << is_key_pressed << std::endl;


	// keyboard 업데이트
	if (false == is_key_pressed)
	{
		keyboard_input_.erase(key);
	}
	else
	{
		keyboard_input_[key] = is_key_pressed;
	}

	XMFLOAT3 animation_vector = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 direction_vector = XMFLOAT3(0.f, 0.f, 0.f);

	// Process keyboard input
	for (const auto& entry : keyboard_input_)
	{
		char key_char = entry.first;
		bool is_pressed = entry.second;

		if (is_pressed)
		{
			switch (key_char)
			{
				// Movement
			case 'W':
				direction_vector = Vector3::Add(direction_vector, { 0, 0, 1 });
				break;
			case 'S':
				direction_vector = Vector3::Add(direction_vector, { 0, 0, 1 });
				break;
			case 'A':
				direction_vector = Vector3::Add(direction_vector, { -1, 0, 0 });
				break;
			case 'D':
				direction_vector = Vector3::Add(direction_vector, { 1, 0, 0 });
				break;
				// 
			case ' ':
				// Jump
				break;
			default:
				break;
			}
		}

	}

	direction_vector_ = direction_vector;
}
