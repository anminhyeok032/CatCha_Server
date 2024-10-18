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
	client_addr_.sin_port = htons(UDPPORT);
	char SERVER_ADDR[BUFSIZE] = "127.0.0.1";
	inet_pton(AF_INET, SERVER_ADDR, &client_addr_.sin_addr);

	//client_addr_.sin_addr = addr.sin_addr;
}

void Player::SendLoginInfoPacket()
{
	SC_LOGIN_INFO_PACKET p;
	p.id = id_;
	p.size = sizeof(p);
	p.type = SC_LOGIN_INFO;
	p.x = position_.x;
	p.y = position_.y;
	p.z = position_.z;
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
		key_ = p->keyinput;
		InputKey();
		break;
	}
	case CS_ROTATE:
	{
		CS_ROTATE_PACKET* p = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		player_pitch_ = p->player_pitch;
		total_pitch_ += player_pitch_;
		//std::cout << "�÷��̾� pitch : " << player_pitch_ << std::endl;
		UpdateRotation(player_pitch_);
		if (dirty_)
		{
			
			UpdateLookUpRight();
			
			// ȸ�� ���� �ʱ�ȭ
			dirty_ = false;

			// ȸ�� ������Ʈ
			commandQueue.push(comp_key_.session_id);
		}
		break;
	}
	case CS_SYNC_PLAYER:
	{
		CS_SYNC_PLAYER_PACKET* p = reinterpret_cast<CS_SYNC_PLAYER_PACKET*>(packet);
		position_ = DirectX::XMFLOAT3(p->x, p->y, p->z);
		m_look = DirectX::XMFLOAT3(p->look_x, p->look_y, p->look_z);

		//std::cout << "�÷��̾� ��ġ : " << position_.x << ", " << position_.y << ", " << position_.z << std::endl;
		//std::cout << "�÷��̾� look : " << m_look.x << ", " << m_look.y << ", " << m_look.z << std::endl;

		// TODO : ���� look vector�� ���ʹϾ� ������Ʈ �ڵ� ����
		

		dirty_ = true;

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
	bool is_moving = false;

	if(prev_player_pitch_ != player_pitch_)
	{
		is_moving = true;
		prev_player_pitch_ = player_pitch_;
	}

	float time_remaining = 0.0f;

	// ���� �ð��� ����
	time_remaining += deltaTime;

	while (time_remaining >= FIXED_TIME_STEP)
	{
		delta_position_ = DirectX::XMFLOAT3();

		// �ӵ� ��� �� �˻�
		if (UpdateVelocity(FIXED_TIME_STEP)) 
		{
			is_moving = true;
		}

		// ��� ������ �� ���� ���� �� �˻�
		ApplyDecelerationIfStop(FIXED_TIME_STEP);

		// �� ����
		ApplyForces(FIXED_TIME_STEP);

		// ���� ����
		ApplyFriction(FIXED_TIME_STEP);

		// �߷� ����
		//ApplyGravity(FIXED_TIME_STEP);

		// ��ġ ������Ʈ
		position_ = MathHelper::Add(position_, delta_position_);

		time_remaining -= FIXED_TIME_STEP;  // ���� �ð� ���ܸ�ŭ ������ �ð��� ���ҽ�Ŵ
	}
	

	return is_moving;
}

void Player::UpdateRotation(float yaw)
{
	DirectX::XMStoreFloat4(&rotation_quat_,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation_quat_),
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yaw / 100.0f)));

	dirty_ = true;
}

void Player::UpdateLookUpRight()
{
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation_quat_));

	m_look = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), rotate_matrix));
	m_up = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), rotate_matrix));
	m_right = MathHelper::Normalize(MathHelper::Cross(GetUp(), GetLook()));
}

bool Player::UpdateVelocity(float time_step) 
{
	speed_ = MathHelper::Length_XZ(GetVelocity());
	if (speed_ > max_speed_) 
	{
		float scale_factor = max_speed_ / speed_;
		velocity_vector_.x *= scale_factor;
		velocity_vector_.y = 0.0f;  // Y�� ����
		velocity_vector_.z *= scale_factor;
		speed_ = max_speed_;
	}
	DirectX::XMFLOAT3 delta = MathHelper::Multiply(GetVelocity(), time_step);
	delta.y = 0.0f;  // Y�� ����
	delta_position_ = MathHelper::Add(delta_position_, delta);

	// �ӵ��� 0���� �˻�
	return speed_ != 0.0f;
}

void Player::ApplyDecelerationIfStop(float time_step)
{
	if (speed_ > 0.0f) 
	{
		float dec = deceleration_ * time_step;
		float new_speed = MathHelper::Max(speed_ - dec, 0.0f);
		if (speed_ > 0) // 0���� �ȳ�������
		{
			float scale_factor = new_speed / speed_;
			velocity_vector_.x *= scale_factor;
			velocity_vector_.z *= scale_factor;
		}
		
	}
}

void Player::ApplyForces(float time_step)
{
	if (false == IsZeroVector(force_vector_))
	{
		DirectX::XMFLOAT3 delta_force = MathHelper::Multiply(GetForce(), time_step);
		delta_force.y = 0.0f;  // Y�� ����
		delta_position_ = MathHelper::Add(delta_position_, delta_force);
	}
}

void Player::ApplyFriction(float time_step) 
{
	float speed = MathHelper::Length(delta_position_);
	if (speed > 0.0f) 
	{
		float dec = deceleration_ * time_step;
		float new_speed = MathHelper::Max(speed - dec, 0.0f);
		if (speed > 0) // 0���� �ȳ�������
		{ 
			float scale_factor = new_speed / speed;
			force_vector_ = MathHelper::Multiply(GetForce(), scale_factor);
		}
	}
}

void Player::ApplyGravity(float time_step)
{
	velocity_vector_.y -= GRAVITY * time_step;
}

void Player::InputKey()
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action key = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard ������Ʈ
	keyboard_input_[key] = is_key_pressed;
	

	XMFLOAT3 input_vector = XMFLOAT3(0.f, 0.f, 0.f);

	// Process keyboard input
	for (const auto& entry : keyboard_input_)
	{
		Action key_char = entry.first;
		bool is_pressed = entry.second;

		if (is_pressed)
		{
			switch (key_char)
			{
				// Movement
			case Action::MOVE_FORWARD:
				Move_Forward();
				break;
			case Action::MOVE_BACK:
				Move_Back();
				break;
			case Action::MOVE_LEFT:
				Move_Left();
				break;
			case Action::MOVE_RIGHT:
				Move_Right();
				break;
			// TODO : Ŭ���̾�Ʈ ���� ���� �� �߰� ����
			//case ' ':
			//	// Jump
			//	if (false == is_jumping_)
			//	{
			//		input_vector = Vector3::Add(input_vector, { 0, 1, 0 });
			//		is_jumping_ = true;
			//	}
			//	break;
			default:
				std::cout << "Invalid key input" << std::endl;
				break;
			}
		}

	}

	// Ű �Է¿� ���� �̵� ������Ʈ
	commandQueue.push(comp_key_.session_id);
}



void Player::Move_Forward() 
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetLook(), acceleration_);
}

void Player::Move_Back() 
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetLook(), -acceleration_);
}

void Player::Move_Left()
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetRight(), -acceleration_);
}

void Player::Move_Right() 
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetRight(), acceleration_);
}