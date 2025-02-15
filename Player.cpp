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
	std::wcout << L" : 에러 : " << msg_buf;
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
		// 로그인 패킷 처리
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(name, p->name);
		strcpy_s(password, p->password);

		// TODO : SQL 연결 작업 추가 필요
		std::cout << name << " login " << std::endl;

		SendLoginInfoPacket(true);

		break;
	}
	case CS_CHOOSE_CHARACTER:
	{
		CS_CHOOSE_CHARACTER_PACKET* p = reinterpret_cast<CS_CHOOSE_CHARACTER_PACKET*>(packet);
		std::cout << "캐릭터 선택 : " << (p->is_cat ? "Cat" : "Mouse") << std::endl;

		{
			std::lock_guard<std::mutex> lg{ mt_player_server_state_ };
			player_server_state_ = PLAYER_STATE::PS_INGAME;


			// [매칭] 새로 들어갈 세션 정보 저장
			int session_id = GetSessionNumber(p->is_cat);
			int client_id = static_cast<int>(g_sessions[session_id].CheckCharacterNum());

			// 새로운 세션으로 임시 세션 플레이어 정보 옮기
			int prev_session_num = *comp_key_.session_id;
			int prev_player_index = *comp_key_.player_index;
			g_sessions[session_id].players_.emplace(client_id, std::move(g_sessions[*comp_key_.session_id].players_[*comp_key_.player_index]));
			*comp_key_.session_id = session_id;
			*comp_key_.player_index = client_id;

			// 캐릭터 선택
			g_sessions[session_id].SetCharacter(session_id, client_id, p->is_cat);
			std::cout << "[ " << name << " ] - " << "[ " << session_id << " ] 세션에 " << client_id << "번째 플레이어로 " << (p->is_cat ? "Cat" : "Mouse") << "로 입장" << std::endl;

			// 새로운 플레이어 receive
			g_sessions[session_id].players_[client_id]->prev_packet_.clear();
			g_sessions[session_id].players_[client_id]->DoReceive();
		}
		
		// 치즈 랜덤 모양 전송
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

		//std::cout << "플레이어 pitch : " << player_pitch_ << std::endl;
		UpdatePitch(player_pitch_);

		if (dirty_)
		{
			UpdateLookUpRight();
			if (p->player_yaw != 0.0f)
			{
				// 고양이 차징 점프용 yaw 변화량 저장
				if (character_state_)
				{
					total_yaw_ += p->player_yaw;
					total_yaw_ = MathHelper::Min(RIGHT_ANGLE_RADIAN - 0.01f, MathHelper::Max(total_yaw_, -RIGHT_ANGLE_RADIAN + 0.01f));
					character_state_->UpdateYaw(this, total_yaw_);
				}

			}

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

		if (character_state_)
		{
			if(false == moveable_ || NUM_CAT == character_id_)
			{
				return;
			}

			keyboard_input_[Action::ACTION_ONE] = true;

			int sessionId = *comp_key_.session_id;

			// look 정보를 통해 해당 위치의 voxel 삭제
			// 캐릭터의 위치 및 방향 벡터
			DirectX::XMFLOAT3 look{ p->look_x, p->look_y, p->look_z };
			DirectX::XMFLOAT3 position = character_state_.get()->GetOBB().Center;
			position.y += (4.0f - (5.104148f / 2.0f));	// 카메라 높이에 맞춰줌
			

			DirectX::XMVECTOR player_position = XMLoadFloat3(&position);
			DirectX::XMVECTOR normal_look = DirectX::XMVector3Normalize(XMLoadFloat3(&look));

			// 공격 OBB의 중심점 설정
			// 캐릭터 앞 공격 범위의 절반만큼 이동
			DirectX::XMVECTOR center_offset = DirectX::XMVectorScale(normal_look, MOUSE_BITE_SIZE);
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

	if (character_state_)
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
		// 차징중에는 위치 고정
		if (jump_charging_time_ > 0.01f)
		{
			velocity_vector_.x = velocity_vector_.z = 0.0f;
		}

		bool moved = false;
		character_state_->ApplyGravity(this, deltaTime);
		// 물리 처리
		moved = character_state_->CalculatePhysics(this, deltaTime);

		// 충돌 처리
		character_state_->CheckIntersects(this, deltaTime);

		// 치즈와의 충돌 처리
		if (true == character_state_->CheckCheeseIntersects(this, deltaTime))
		{
			moved = true;
		}

		// 최종 위치 동기화
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
	curr_hp_ = 100;					// 현재 체력
	reborn_ai_character_id_ = -1;	// 부활시킬 AI 캐릭터 번호

	// physics
	speed_ = 0.0f;				// 현재 속도

	// 애니메이션 동기화 관련 변수들
	on_ground_ = false;
	obj_state_ = Object_State::STATE_IDLE;
	// 움직임 업데이트가 필요할때
	force_move_update_ = false;

	moveable_ = true;
	stop_skill_time_ = 0.0f;

	direction_vector_ = DirectX::XMFLOAT3();
	velocity_vector_ = DirectX::XMFLOAT3();
	force_vector_ = DirectX::XMFLOAT3();
	depth_delta_ = DirectX::XMFLOAT3();

	player_pitch_ = 0.0f;
	prev_player_pitch_ = 0.0f;
	// 받은 pitch 변화값 총량
	total_pitch_ = 0;
	total_yaw_ = 0;

	rotation_quat_ = { 0, 0, 0, 1 };						// 초기 쿼터니언 (단위 쿼터니언)
	rotation_matrix_ = MathHelper::Identity_4x4();		// 회전 행렬

	look_ = { 0.0f, 0.0f, 1.0f };
	up_ = { 0.0f, 1.0f, 0.0f };
	right_ = { 1.0f, 0.0f, 0.0f };

	dirty_ = false;  // 회전 상태가 변경되었는지 확인

	delta_position_ = DirectX::XMFLOAT3();

	character_state_.reset();

	// 플레이어 업데이트 여부
	needs_update_.store(false);

	// bite시 생기는 구의 중점
	bite_center_ = DirectX::XMFLOAT3();

	// 결과 조건 초기화
	request_send_escape_ = false;
	request_send_dead_ = false;
	request_send_reborn_ = false;

	keyboard_input_.clear();
	key_ = 0;
}
