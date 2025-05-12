#include "Player.h"
#include "CharacterState.h"
#include "AIPlayer.h"

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

void Player::SetState(std::unique_ptr<CharacterState> new_state)
{
	character_state_ = std::move(new_state);
}

void Player::DoReceive()
{
	DWORD recv_flag = 0;
	memset(&recv_over_.over_, 0, sizeof(recv_over_.over_));

	recv_over_.wsabuf_.buf = recv_over_.send_buf_ + prev_packet_.size();
	DWORD remaining_size = static_cast<DWORD>(BUF_SIZE - prev_packet_.size());
	recv_over_.wsabuf_.len = remaining_size;

	WSARecv(socket_, &recv_over_.wsabuf_, 1, 0, &recv_flag, &recv_over_.over_, 0);
}

void Player::DoSend(void* packet)
{
	Over_IO* send_over = new Over_IO {reinterpret_cast<unsigned char*>(packet), SOCKET_TYPE::TCP_SOCKET};
	int res = WSASend(socket_, &send_over->wsabuf_, 1, 0, 0, &send_over->over_, 0);
}

void Player::SendLoginInfoPacket(bool result)
{
	SC_LOGIN_INFO_PACKET p;
	p.size = sizeof(p);
	p.type = SC_LOGIN_INFO;
	p.result = result;
	DoSend(&p);
}

void Player::SendMyPlayerNumber()
{ 
	SC_SET_MY_ID_PACKET p;
	p.size = sizeof(p);
	p.type = SC_SET_MY_ID;
	p.my_id = *comp_key_.player_index;
	DoSend(&p);
}

void Player::SendRandomCheeseSeedPacket()
{
	SC_RANDOM_VOXEL_SEED_PACKET p;
	p.size = sizeof(p);
	p.type = SC_RANDOM_VOXEL_SEED;
	for (int i = 0; i < CHEESE_NUM; ++i)
	{
		p.random_seeds[i] = g_voxel_pattern_manager.random_seeds_[i];
	}
	DoSend(&p);
}

// IO thread
void Player::ProcessPacket(char* packet)
{
	switch (packet[1])
	{
		// �α��� ��Ŷ ó��
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(name, p->name);
		strcpy_s(password, p->password);

		// TODO : SQL ���� �۾� �߰� �ʿ�
		std::cout << name << " login " << std::endl;

		SendLoginInfoPacket(true);

		break;
	}
	case CS_CHOOSE_CHARACTER:
	{
		CS_CHOOSE_CHARACTER_PACKET* p = reinterpret_cast<CS_CHOOSE_CHARACTER_PACKET*>(packet);
		std::cout << "ĳ���� ���� : " << (p->is_cat ? "Cat" : "Mouse") << std::endl;

		{
			std::lock_guard<std::mutex> lg{ mt_player_server_state_ };
			player_server_state_ = PLAYER_STATE::PS_INGAME;


			// [��Ī] ���� �� ���� ���� ����
			int session_id = GetSessionNumber(p->is_cat);
			int client_id = static_cast<int>(g_sessions[session_id].CheckCharacterNum());

			// ���ο� �������� �ӽ� ���� �÷��̾� ���� �ű�
			int prev_session_num = *comp_key_.session_id;
			int prev_player_index = *comp_key_.player_index;
			g_sessions[session_id].players_.emplace(client_id, std::move(g_sessions[*comp_key_.session_id].players_[*comp_key_.player_index]));
			*comp_key_.session_id = session_id;
			*comp_key_.player_index = client_id;

			// ĳ���� ����
			g_sessions[session_id].SetCharacter(session_id, client_id, p->is_cat);
			std::cout << "[ " << name << " ] - " << "[ " << session_id << " ] ���ǿ� " << client_id << "��° �÷��̾�� " << (p->is_cat ? "Cat" : "Mouse") << "�� ����" << std::endl;

			// ���ο� �÷��̾� receive
			g_sessions[session_id].players_[client_id]->prev_packet_.clear();
			g_sessions[session_id].players_[client_id]->DoReceive();
		}
		
		// ġ�� ���� ��� ����
		SendRandomCheeseSeedPacket();
		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		key_ = p->keyinput;
		if (character_state_)
		{
			character_state_->InputKey(this, key_);
		}
		break;
	}
	case CS_ROTATE:
	{
		CS_ROTATE_PACKET* p = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		player_pitch_ = p->player_pitch;
		total_pitch_ += player_pitch_;

		//std::cout << "�÷��̾� pitch : " << player_pitch_ << std::endl;
		UpdatePitch(player_pitch_);

		if (dirty_)
		{
			UpdateLookUpRight();
			if (p->player_yaw != 0.0f)
			{
				// ����� ��¡ ������ yaw ��ȭ�� ����
				if (character_state_)
				{
					total_yaw_ += p->player_yaw;
					total_yaw_ = MathHelper::Min(RIGHT_ANGLE_RADIAN - 0.01f, MathHelper::Max(total_yaw_, -RIGHT_ANGLE_RADIAN + 0.01f));
					character_state_->UpdateYaw(this, total_yaw_);
				}

			}

			// ȸ�� ���� �ʱ�ȭ
			dirty_ = false;

			// �÷��̾� ������Ʈ
			RequestUpdate();
			
		}
		break;
	}
	case CS_VOXEL_LOOK:
	{
		CS_VOXEL_LOOK_PACKET* p = reinterpret_cast<CS_VOXEL_LOOK_PACKET*>(packet);

		if (character_state_)
		{
			if(false == moveable_ || NUM_CAT == character_id_)
			{
				return;
			}

			keyboard_input_[Action::ACTION_ONE] = true;

			int sessionId = *comp_key_.session_id;

			// look ������ ���� �ش� ��ġ�� voxel ����
			// ĳ������ ��ġ �� ���� ����
			DirectX::XMFLOAT3 look{ p->look_x, p->look_y, p->look_z };
			DirectX::XMFLOAT3 position = character_state_.get()->GetOBB().Center;
			position.y += (4.0f - (5.104148f / 2.0f));	// ī�޶� ���̿� ������
			

			DirectX::XMVECTOR player_position = XMLoadFloat3(&position);
			DirectX::XMVECTOR normal_look = DirectX::XMVector3Normalize(XMLoadFloat3(&look));

			// ���� OBB�� �߽��� ����
			// ĳ���� �� ���� ������ ���ݸ�ŭ �̵�
			DirectX::XMVECTOR center_offset = DirectX::XMVectorScale(normal_look, MOUSE_BITE_SIZE);
			DirectX::XMVECTOR attack_center = DirectX::XMVectorAdd(player_position, center_offset);
			DirectX::XMStoreFloat3(&position, attack_center);

			// �ش� ��ġ ������, �ش� ���� ������Ʈ�� �������� - cheese ���� ������� �ʱ� ����
			bite_center_ = position;
			// ������Ʈ ��û
			RequestUpdate();
		}
		break;
	}
	case CS_SYNC_PLAYER:
	{
		CS_SYNC_PLAYER_PACKET* p = reinterpret_cast<CS_SYNC_PLAYER_PACKET*>(packet);
		position_ = DirectX::XMFLOAT3(p->x, p->y, p->z);
		rotation_quat_ = DirectX::XMFLOAT4(p->quat_x, p->quat_y, p->quat_z, p->quat_w);

		dirty_ = true;

		break;
	}
	case CS_TIME:
	{
		CS_TIME_PACKET* p = reinterpret_cast<CS_TIME_PACKET*>(packet);
		//remaining_time_ = p->time;
		//std::cout << "���� �ð� : " << remaining_time_ << std::endl;
		break;
	}
	default:
	{
		std::cout << "Invalid packet : error" << std::endl;
		break;
	}
	}
}

//=================================================================================================
// Update thread ����
//=================================================================================================
bool Player::UpdatePosition(float deltaTime)
{
	// �÷��̾� ������Ʈ ��û ����
	needs_update_.store(false);

	if (character_state_)
	{
		// ��ų ������� �ƴϸ�
		if (moveable_ == true)
		{
			// TODO : ����̿� ���� ó���� ������ ����
			// ù �ٿ�� Ű�� ���� �̵� ó��
			for (const auto& key : keyboard_input_)
			{
				if (key.second)
				{
					switch (key.first)
					{
					case Action::MOVE_FORWARD:
						MoveForward();
						break;
					case Action::MOVE_BACK:
						MoveBack();
						break;
					case Action::MOVE_LEFT:
						MoveLeft();
						break;
					case Action::MOVE_RIGHT:
						MoveRight();
						break;
					case Action::ACTION_JUMP:
						character_state_->Jump(this);
						break;
					case Action::ACTION_ONE:
						keyboard_input_[Action::ACTION_ONE] = false;
						character_state_->ActionOne(this);
						break;
					case Action::ACTION_FOUR:
						character_state_->ActionFourCharging(this, deltaTime);
						break;
					case Action::ACTION_FIVE:
						keyboard_input_[Action::ACTION_FIVE] = false;
						character_state_->ChargingJump(this, jump_charging_time_);
						break;
					default:
						break;
					}
				}
			}
		}
		if (deltaTime > UPDATE_PERIOD * 3.0f)
		{
			deltaTime = UPDATE_PERIOD * 2.0f;
		}
		// ��¡�߿��� ��ġ ����
		if (jump_charging_time_ > 0.01f)
		{
			velocity_vector_.x = velocity_vector_.z = 0.0f;
		}

		bool moved = false;
		character_state_->ApplyGravity(this, deltaTime);
		// ���� ó��
		moved = character_state_->CalculatePhysics(this, deltaTime);

		// �浹 ó��
		character_state_->CheckIntersects(this, deltaTime);

		// ġ����� �浹 ó��
		if (true == character_state_->CheckCheeseIntersects(this, deltaTime))
		{
			moved = true;
		}

		// ���� ��ġ ����ȭ
		if (true == character_state_->CalculatePosition(this, deltaTime))
		{
			moved = true;
		}

		character_state_->UpdateOBB(this);

		if (true == force_move_update_)
		{
			force_move_update_ = false;
			moved = true;
		}

		// OBB ���� �� �ٽ� ������Ʈ ��û
		if (moved)
		{
			// �÷��̾� ������Ʈ ��û
			RequestUpdate();
		}
		return moved;
	}
	return false;
}

void Player::UpdatePitch(float degree)
{
	DirectX::XMStoreFloat4(&rotation_quat_,
		DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation_quat_),
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), degree)));

	dirty_ = true;
}

void Player::UpdateLookUpRight()
{
	DirectX::XMMATRIX rotate_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation_quat_));

	look_ = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), rotate_matrix));
	up_ = MathHelper::Normalize(MathHelper::Multiply(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), rotate_matrix));
	right_ = MathHelper::Normalize(MathHelper::Cross(GetUp(), GetLook()));
}

bool Player::UpdateVelocity(float time_step) 
{
	speed_ = MathHelper::Length_XZ(GetVelocity());
	if (speed_ > max_speed_) 
	{
		float scale_factor = max_speed_ / speed_;
		velocity_vector_.x *= scale_factor;
		velocity_vector_.z *= scale_factor;
		speed_ = max_speed_;
	}
	DirectX::XMFLOAT3 delta = MathHelper::Multiply(GetVelocity(), time_step);
	delta_position_ = MathHelper::Add(delta_position_, delta);

	// �ӵ��� 0���� �˻�
	return speed_ != 0.0f || velocity_vector_.y != 0.0f;
}

void Player::ApplyDecelerationIfStop(float time_step)
{
	if (speed_ > 0.0f) 
	{
		float dec = deceleration_ * time_step;
		float new_speed = MathHelper::Max(speed_ - dec, 0.0f);
		if (speed_ > 0.0f) // 0���� �ȳ�������
		{
			float scale_factor = new_speed / speed_;
			velocity_vector_.x *= scale_factor;
			//velocity_vector_.y *= scale_factor;
			velocity_vector_.z *= scale_factor;
		}
		
	}
}

void Player::ApplyForces(float time_step)
{
	if (false == IsZeroVector(force_vector_))
	{
		DirectX::XMFLOAT3 delta_force = MathHelper::Multiply(GetForce(), time_step);
		//delta_force.y = 0.0f;  // Y�� ����
		delta_position_ = MathHelper::Add(delta_position_, delta_force);
	}
}

void Player::ApplyFriction(float time_step) 
{
	DirectX::XMFLOAT3 delta = MathHelper::Multiply(GetForce(), time_step);
	float force = MathHelper::Length_XZ(delta);

	if (true == on_ground_) {
		if (force > 0.0f) {
			float friction = FRICTION * time_step;
			float new_force = MathHelper::Max(force - friction, 0.0f);

			DirectX::XMFLOAT3 calculated_force = MathHelper::Multiply(GetForce(), new_force / force);
			force_vector_.x = calculated_force.x;
			//force_vector_.y = calculated_force.y;
			force_vector_.z = calculated_force.z;
		}
	}
}

void Player::Set_OBB(DirectX::BoundingOrientedBox obb)
{
	position_ = obb.Center;
	position_.y -= obb.Extents.y;
	rotation_quat_ = obb.Orientation;
}


void Player::MoveForward()
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetLook(), acceleration_);
}

void Player::MoveBack() 
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetLook(), -acceleration_);
}

void Player::MoveLeft()
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetRight(), -acceleration_);
}

void Player::MoveRight() 
{
	velocity_vector_ = MathHelper::Add(GetVelocity(), GetRight(), acceleration_);
}

void Player::ResetPlayer()
{
	character_id_ = NUM_GHOST;
	curr_hp_ = 100;					// ���� ü��
	reborn_ai_character_id_ = -1;	// ��Ȱ��ų AI ĳ���� ��ȣ

	// physics
	speed_ = 0.0f;				// ���� �ӵ�

	// �ִϸ��̼� ����ȭ ���� ������
	on_ground_ = false;
	obj_state_ = Object_State::STATE_IDLE;
	// ������ ������Ʈ�� �ʿ��Ҷ�
	force_move_update_ = false;

	moveable_ = true;
	stop_skill_time_ = 0.0f;

	direction_vector_ = DirectX::XMFLOAT3();
	velocity_vector_ = DirectX::XMFLOAT3();
	force_vector_ = DirectX::XMFLOAT3();
	depth_delta_ = DirectX::XMFLOAT3();

	player_pitch_ = 0.0f;
	prev_player_pitch_ = 0.0f;
	// ���� pitch ��ȭ�� �ѷ�
	total_pitch_ = 0;
	total_yaw_ = 0;

	rotation_quat_ = { 0, 0, 0, 1 };						// �ʱ� ���ʹϾ� (���� ���ʹϾ�)
	rotation_matrix_ = MathHelper::Identity_4x4();		// ȸ�� ���

	look_ = { 0.0f, 0.0f, 1.0f };
	up_ = { 0.0f, 1.0f, 0.0f };
	right_ = { 1.0f, 0.0f, 0.0f };

	dirty_ = false;  // ȸ�� ���°� ����Ǿ����� Ȯ��

	delta_position_ = DirectX::XMFLOAT3();

	character_state_.reset();

	// �÷��̾� ������Ʈ ����
	needs_update_.store(false);

	// bite�� ����� ���� ����
	bite_center_ = DirectX::XMFLOAT3();

	// ��� ���� �ʱ�ȭ
	request_send_escape_ = false;
	request_send_dead_ = false;
	request_send_reborn_ = false;

	keyboard_input_.clear();
	key_ = 0;
}
