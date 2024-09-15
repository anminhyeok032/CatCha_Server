#include "Player.h"

void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : 에러 : " << msg_buf;
	while (true);
	LocalFree(msg_buf);
}

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
	Over_IO* send_over = new Over_IO {reinterpret_cast<unsigned char*>(packet), SOCKET_TYPE::TCP_SOCKET};
	int res = WSASend(socket_, &send_over->wsabuf_, 1, 0, 0, &send_over->over_, 0);
}

void Player::SetAddr() {
	// TCP 소켓을 통해 얻은 클라이언트의 주소를 저장
	sockaddr_in addr;
	int addrLen = sizeof(addr);

	// TCP 소켓의 주소
	getpeername(socket_, (sockaddr*)&addr, &addrLen);

	// UDP 소켓 주소로 저장
	client_addr_.sin_family = AF_INET;
	client_addr_.sin_port = htons(PORT);
	client_addr_.sin_addr = addr.sin_addr;
}

void Player::SendLoginInfoPacket()
{
	SC_LOGIN_INFO_PACKET p;
	p.id = id_;
	p.size = sizeof(p);
	p.type = SC_LOGIN_INFO;
	p.x = x_;
	p.y = y_;
	p.z = z_;
	DoSend(&p);
}

void Player::ProcessPacket(char* packet)
{

	switch (packet[1])
	{
		// 로그인 패킷 처리
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		std::cout << p->name << " login " << std::endl;
		
		SendLoginInfoPacket();
		SetAddr();
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

bool Player::UpdatePosition(float deltaTime)
{
	// TODO : 마우스를 이용한 방향 벡터는 아직 고려되지 않음
	// 현재 반암묵적 오일러를 이용한 위치 업데이트 사용중
	// 클라이언트와 동기화가 지속적으로 안맞을시 Runge-Kutta 참고해서 수정할 것

	if (true == IsZeroVector(direction_vector_))
	{
		// 입력이 없으면 마찰력만 적용시켜 속도 급감소
		velocity_vector_ = velocity_vector_ * std::pow(1.0f - FRICTION, deltaTime);

		// 속도가 STOP_THRESHOLD보다 작아지면 0으로 설정
		if (std::fabs(velocity_vector_.x) < STOP_THRESHOLD) velocity_vector_.x = 0;
		if (std::fabs(velocity_vector_.y) < STOP_THRESHOLD) velocity_vector_.y = 0;
		if (std::fabs(velocity_vector_.z) < STOP_THRESHOLD) velocity_vector_.z = 0;

		// 남은 velocity 없으면 해당 플레이어 업데이트 false
		if (true == IsZeroVector(velocity_vector_))
		{
			return false;
		}
	}
	else
	{
		// 입력이 있을 때는 일정 속도로 움직임
		velocity_vector_ = velocity_vector_ + (direction_vector_ * 500.f);
		// 마찰력 적용
		velocity_vector_ = velocity_vector_ * std::pow(1.0f - FRICTION, deltaTime);
	}

	// 중력 적용 
	if (true == is_jumping_)
	{
		velocity_vector_.y -= GRAVITY * deltaTime * 10.f;
	}

	// 속도 업데이트 ( velocity 먼저 반영한 반암묵적 오일러)
	x_ += velocity_vector_.x * deltaTime;
	y_ += velocity_vector_.y * deltaTime;
	z_ += velocity_vector_.z * deltaTime;

	// 바닥에 닿았을 때
	if (y_ <= 0)
	{
		y_ = 0;
		is_jumping_ = false;
		velocity_vector_.y = 0; // 수직 속도를 0으로 설정하여 떨어지지 않게 함
	}

	// TODO : 충돌 감지 및 처리 필요함
	// CollisionCheck();

	return true;
}



void Player::InputKey()
{
	bool is_key_pressed = Direction & 0x01;
	uint8_t key_stroke = Direction >> 1;
	char key = static_cast<char>(key_stroke);

	std::cout << "key input : " << key << " = " << (is_key_pressed ? "true" : "false") << std::endl;


	// keyboard 업데이트
	if (false == is_key_pressed)
	{
		keyboard_input_.erase(key);
	}
	else
	{
		keyboard_input_[key] = is_key_pressed;
	}

	XMFLOAT3 input_vector = XMFLOAT3(0.f, 0.f, 0.f);

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
				input_vector = Vector3::Add(input_vector, { 0, 0, 1 });
				break;
			case 'S':
				input_vector = Vector3::Add(input_vector, { 0, 0, -1 });
				break;
			case 'A':
				input_vector = Vector3::Add(input_vector, { -1, 0, 0 });
				break;
			case 'D':
				input_vector = Vector3::Add(input_vector, { 1, 0, 0 });
				break;
				// 
			case ' ':
				// Jump
				if (false == is_jumping_)
				{
					input_vector = Vector3::Add(input_vector, { 0, 1, 0 });
					is_jumping_ = true;
				}
				break;
			default:
				break;
			}
		}

	}

	// 방향 벡터 업데이트
	direction_vector_ = Vector3::Normalize(input_vector);
	commandQueue.push(comp_key_.session_id);
}
