#include "Player.h"
#include "CharacterState.h"

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

void Player::SetState(std::unique_ptr<CharacterState> new_state)
{
	state_ = std::move(new_state);
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

void Player::SendLoginInfoPacket()
{
	SC_LOGIN_INFO_PACKET p;
	p.id = comp_key_.player_index;
	p.size = sizeof(p);
	p.type = SC_LOGIN_INFO;
	p.x = position_.x;
	p.y = position_.y;
	p.z = position_.z;
	DoSend(&p);
}

// IO thread
void Player::ProcessPacket(char* packet)
{
	switch (packet[1])
	{
		// 로그인 패킷 처리
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		std::cout << p->name << " login " << std::endl;
		std::cout << "플레이어 번호 : " << comp_key_.player_index << std::endl;

		SendLoginInfoPacket();
		break;
	}
	case CS_CHOOSE_CHARACTER:
	{
		CS_CHOOSE_CHARACTER_PACKET* p = reinterpret_cast<CS_CHOOSE_CHARACTER_PACKET*>(packet);
		std::cout << "캐릭터 선택 : " << (p->is_cat ? "Cat" : "Mouse") << std::endl;
		int sessionId = comp_key_.session_id;
		int playerIndex = comp_key_.player_index;
		
		g_sessions[sessionId].SetCharacter(sessionId, playerIndex, p->is_cat);
		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		key_ = p->keyinput;
		if (state_)
		{
			state_->InputKey(this, key_);
		}
		break;
	}
	case CS_ROTATE:
	{
		CS_ROTATE_PACKET* p = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		player_pitch_ = p->player_pitch;
		total_pitch_ += player_pitch_;
		//std::cout << "플레이어 pitch : " << player_pitch_ << std::endl;
		UpdateRotation(player_pitch_);
		if (dirty_)
		{

			UpdateLookUpRight();

			// 회전 상태 초기화
			dirty_ = false;

			// 플레이어 업데이트
			RequestUpdate();
			
		}
		break;
	}
	case CS_VOXEL_LOOK:
	{
		CS_VOXEL_LOOK_PACKET* p = reinterpret_cast<CS_VOXEL_LOOK_PACKET*>(packet);

		if (state_)
		{
			if(false == moveable_ || NUM_CAT == id_)
			{
				return;
			}

			keyboard_input_[Action::ACTION_ONE] = true;

			int sessionId = comp_key_.session_id;

			// look 정보를 통해 해당 위치의 voxel 삭제
			// 캐릭터의 위치 및 방향 벡터
			DirectX::XMFLOAT3 look{ p->look_x, p->look_y, p->look_z };
			DirectX::XMFLOAT3 position = state_.get()->GetOBB().Center;

			DirectX::XMVECTOR player_position = XMLoadFloat3(&position);
			DirectX::XMVECTOR normal_look = DirectX::XMVector3Normalize(XMLoadFloat3(&look));

			// 공격 OBB의 중심점 설정
			// 캐릭터 앞 공격 범위의 절반만큼 이동
			DirectX::XMVECTOR center_offset = DirectX::XMVectorScale(normal_look, 5.0f);
			DirectX::XMVECTOR attack_center = DirectX::XMVectorAdd(player_position, center_offset);
			DirectX::XMStoreFloat3(&position, attack_center);

			// 해당 위치 저장후, 해당 세션 업데이트때 순차적용 - cheese 락을 사용하지 않기 위해
			bite_center_ = position;
			// 업데이트 요청
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
		//std::cout << "게임 시간 : " << remaining_time_ << std::endl;
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
// Update thread 로직
//=================================================================================================
bool Player::UpdatePosition(float deltaTime)
{
	// 플레이어 업데이트 요청 제거
	needs_update_.store(false);

	if (state_)
	{
		// 스킬 사용중이 아니면
		if (moveable_ == true)
		{
			// TODO : 고양이와 쥐의 처리를 나눠서 구현
			// 첫 다운된 키에 따른 이동 처리
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
						state_->Jump(this);
						break;
					case Action::ACTION_ONE:
						keyboard_input_[Action::ACTION_ONE] = false;
						state_->ActionOne(this);
						break;
					default:
						break;
					}
				}
			}
		}

		// 물리처리 - 움직였다면 true 반환
		// 충돌 처리
		state_->CheckIntersects(this, deltaTime);

		// 치즈와의 충돌 처리
		bool moved = state_->CheckCheeseIntersects(this, deltaTime);

		// 물리 처리
		if (true == state_->CalculatePhysics(this, deltaTime))
		{
			 moved = true;
		}

		state_->UpdateOBB(this);

		if (true == force_move_update_)
		{
			force_move_update_ = false;
			moved = true;
		}

		// OBB 갱신 및 다시 업데이트 요청
		if (moved)
		{
			// 플레이어 업데이트 요청
			RequestUpdate();
		}
		return moved;
	}
	return false;
}

void Player::UpdateRotation(float degree)
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
		//velocity_vector_.y = 0.0f;  // Y축 제거
		velocity_vector_.z *= scale_factor;
		speed_ = max_speed_;
	}
	DirectX::XMFLOAT3 delta = MathHelper::Multiply(GetVelocity(), time_step);
	//delta.y = 0.0f;  // Y축 제거
	delta_position_ = MathHelper::Add(delta_position_, delta);
	//delta_position_ = MathHelper::Add(delta_position_, GetVelocity());

	// 떨어질때, 점프 idle로 변환
	// y가 아래로 향할때 && 점프시작이 아니다 && 현재 idle||move 상태라면
	if (velocity_vector_.y < -0.0001f && obj_state_ != Object_State::STATE_JUMP_START 
		&& (obj_state_ == Object_State::STATE_IDLE || obj_state_ == Object_State::STATE_MOVE))
	{
		// 공중에 뜬 상태 && 애니메이션 jump_idle로 변경
		on_ground_= false;
		obj_state_ = Object_State::STATE_JUMP_IDLE;
	}

	// 제자리 점프시 업데이트를 위해 추가
	if (obj_state_ == Object_State::STATE_JUMP_END)
	{
		return true;
	}

	// 속도가 0인지 검사
	return speed_ != 0.0f || velocity_vector_.y != 0.0f;
}

void Player::ApplyDecelerationIfStop(float time_step)
{
	if (speed_ > 0.0f) 
	{
		float dec = deceleration_ * time_step;
		float new_speed = MathHelper::Max(speed_ - dec, 0.0f);
		if (speed_ > 0.0f) // 0으로 안나눠지게
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
		//delta_force.y = 0.0f;  // Y축 제거
		delta_position_ = MathHelper::Add(delta_position_, delta_force);
	}
}

void Player::ApplyFriction(float time_step) 
{
	float speed = MathHelper::Length(MathHelper::Multiply(GetForce(), time_step));
	if (speed > 0.0f) 
	{
		float dec = deceleration_ * time_step;
		
		if (speed > 0.0f) // 0으로 안나눠지게
		{
			float new_speed = MathHelper::Max(speed - dec, 0.0f);
			float scale_factor = new_speed / speed;
			force_vector_ = MathHelper::Multiply(GetForce(), scale_factor);
			//force_vector_.x *= scale_factor;
			//force_vector_.y *= scale_factor;
			//force_vector_.z *= scale_factor;
		}
	}
}

void Player::ApplyGravity(float time_step)
{
	if (velocity_vector_.y > -300.0f)
	{
		velocity_vector_.y -= GRAVITY * time_step;
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
