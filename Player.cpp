#include "Player.h"

void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : ���� : " << msg_buf;
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
	// TCP ������ ���� ���� Ŭ���̾�Ʈ�� �ּҸ� ����
	sockaddr_in addr;
	int addrLen = sizeof(addr);

	// TCP ������ �ּ�
	getpeername(socket_, (sockaddr*)&addr, &addrLen);

	// UDP ���� �ּҷ� ����
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
		// �α��� ��Ŷ ó��
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
		std::cout << "���� �ð� : " << remaining_time_ << std::endl;
		break;
	}
	}
}

bool Player::UpdatePosition(float deltaTime)
{
	// TODO : ���콺�� �̿��� ���� ���ʹ� ���� ������� ����
	// ���� �ݾϹ��� ���Ϸ��� �̿��� ��ġ ������Ʈ �����
	// Ŭ���̾�Ʈ�� ����ȭ�� ���������� �ȸ����� Runge-Kutta �����ؼ� ������ ��

	if (true == IsZeroVector(direction_vector_))
	{
		// �Է��� ������ �����¸� ������� �ӵ� �ް���
		velocity_vector_ = velocity_vector_ * std::pow(1.0f - FRICTION, deltaTime);

		// �ӵ��� STOP_THRESHOLD���� �۾����� 0���� ����
		if (std::fabs(velocity_vector_.x) < STOP_THRESHOLD) velocity_vector_.x = 0;
		if (std::fabs(velocity_vector_.y) < STOP_THRESHOLD) velocity_vector_.y = 0;
		if (std::fabs(velocity_vector_.z) < STOP_THRESHOLD) velocity_vector_.z = 0;

		// ���� velocity ������ �ش� �÷��̾� ������Ʈ false
		if (true == IsZeroVector(velocity_vector_))
		{
			return false;
		}
	}
	else
	{
		// �Է��� ���� ���� ���� �ӵ��� ������
		velocity_vector_ = velocity_vector_ + (direction_vector_ * 500.f);
		// ������ ����
		velocity_vector_ = velocity_vector_ * std::pow(1.0f - FRICTION, deltaTime);
	}

	// �߷� ���� 
	if (true == is_jumping_)
	{
		velocity_vector_.y -= GRAVITY * deltaTime * 10.f;
	}

	// �ӵ� ������Ʈ ( velocity ���� �ݿ��� �ݾϹ��� ���Ϸ�)
	x_ += velocity_vector_.x * deltaTime;
	y_ += velocity_vector_.y * deltaTime;
	z_ += velocity_vector_.z * deltaTime;

	// �ٴڿ� ����� ��
	if (y_ <= 0)
	{
		y_ = 0;
		is_jumping_ = false;
		velocity_vector_.y = 0; // ���� �ӵ��� 0���� �����Ͽ� �������� �ʰ� ��
	}

	// TODO : �浹 ���� �� ó�� �ʿ���
	// CollisionCheck();

	return true;
}



void Player::InputKey()
{
	bool is_key_pressed = Direction & 0x01;
	uint8_t key_stroke = Direction >> 1;
	char key = static_cast<char>(key_stroke);

	std::cout << "key input : " << key << " = " << (is_key_pressed ? "true" : "false") << std::endl;


	// keyboard ������Ʈ
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

	// ���� ���� ������Ʈ
	direction_vector_ = Vector3::Normalize(input_vector);
	commandQueue.push(comp_key_.session_id);
}
