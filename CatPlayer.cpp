#include "CatPlayer.h"
#include "Player.h"
#include "MapData.h"

void CatPlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

    // 스킬 시전중이 아닌 경우에만 스킬 키 이벤트 처리
    if (player->moveable_ == true)
    {
        switch (action)
        {
        case Action::ACTION_ONE:
            ActionOne(player);
            break;
        default:
            break;
        }
    }

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard 업데이트
	player->SetKeyState(action, is_key_pressed);

	// 세션 업데이트 요청
	player->RequestUpdate();
}


void CatPlayer::CheckIntersects(Player* player, float deltaTime)
{
    // 1. bounding sphere를 이용해 검사 범위 축소
    // 2. 현재 위치에 대한 OBB 충돌 검사 (xz축, y축 분리 검사)
    // 3. 충돌 시, 충돌된 물체의 점을 이용해 삼각형 만들고 삼각형과 각 중점사이의 벡터의 교차 검사 및 깊이 검사
    // 4. 찾은 삼각형의 노멀벡터를 이용해서 velocity 투영해서 캐릭터의 velocity 벡터를 조정
    // 5. 얻은 깊이값을 이용해 물체를 뚫지 않도록 위치 조정

    player->ApplyGravity(deltaTime);

    DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR slide_vector = DirectX::XMLoadFloat3(&player->velocity_vector_);


    // 모든 오브젝트 충돌체크
    for (const auto& object : g_obbData)
    {
        // slidevector가 0이면 충돌체크 불필요
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(slide_vector)) < 0.0001f)
        {
            break;
        }

        // 충돌 체크 횟수를 줄이기 위한 BoundingSphere로 먼저 체크
        if (false == player_sphere_.Intersects(object.second.obb))
        {
            continue;
        }

        // 가장 거리가 짧은 충돌 지점까지의 거리
        float depth_ = FLT_MIN;
        // 가장 가까운 면 normal 벡터
        DirectX::XMVECTOR closest_normal = DirectX::XMVectorZero();
       
        if(false == object.second.obb.Intersects(obb_))
        {
			continue;
		}

        //std::cout << "충돌함 : " << object.first << std::endl;

        //가장 거리가 짧은 충돌 지점까지의 거리
        float min_distance = FLT_MAX;


        // 부딪힌 물체 OBB 꼭짓점 배열
        DirectX::XMFLOAT3 corners[8];
        object.second.obb.GetCorners(corners);

        // **1. 위/아래 면 검사**
        // velocity의 y축만 사용
        DirectX::XMVECTOR d = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&object.second.obb.Center), currentPos);
        DirectX::XMVECTOR check_d = DirectX::XMVectorSet(0.0f, DirectX::XMVectorGetY(d), 0.0f, 0.0f);
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(check_d)) > 0.0001f)
        {
            DirectX::XMVECTOR normalized_d = DirectX::XMVector3Normalize(check_d);
            // OBB를 구성하는 각 면(삼각형으로 나눠서 면당 2번)에 대해 충돌 검사
            for (int i = 0; i < 4; ++i) // 위(4~7)와 아래(0~3) 면만 검사
            {
                // 삼각형을 구성하는 각 점을 로드
                const auto& indices = g_triangle_indices[i];
                DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&corners[indices[0]]);
                DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&corners[indices[1]]);
                DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&corners[indices[2]]);

                // 삼각형과 velocity의 교차 여부 확인
                float t;    // 충돌 지점까지의 거리
                // 삼각형과 velocity 벡터가 교차하는지 확인
                bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_d, p1, p2, p3, t);

                // 교차 && 가장 가까운 거리인지 확인
                if (intersects && t < min_distance)
                {
                    min_distance = t;
                    // 가장 가까운 면의 normal 벡터 계산
                    closest_normal = DirectX::XMVector3Normalize(
                        DirectX::XMVector3Cross(
                            DirectX::XMVectorSubtract(p2, p1), DirectX::XMVectorSubtract(p3, p1)
                        )
                    );
                }
            }
        }

        // **2. 옆면 검사 (앞, 뒤, 좌, 우)**
        // velocity의 xz만 사용
        check_d = DirectX::XMVectorSet(
            DirectX::XMVectorGetX(d),
            0.0f,
            DirectX::XMVectorGetZ(d),
            0.0f);
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(check_d)) > 0.0001f)
        {
            DirectX::XMVECTOR normalized_d = DirectX::XMVector3Normalize(check_d);
            for (int i = 4; i < 12; ++i)
            {
                const auto& indices = g_triangle_indices[i];
                DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&corners[indices[0]]);
                DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&corners[indices[1]]);
                DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&corners[indices[2]]);

                // 삼각형과 velocity의 교차 여부 확인
                float t;
                bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_d, p1, p2, p3, t);

                if (intersects && t < min_distance)
                {
                    min_distance = t;
                    closest_normal = DirectX::XMVector3Normalize(
                        DirectX::XMVector3Cross(
                            DirectX::XMVectorSubtract(p2, p1), DirectX::XMVectorSubtract(p3, p1)
                        )
                    );
                }
            }
        }
        
        // 스치는 경우 충돌이 안된 판정이 되어버림
        if(DirectX::XMVectorGetX(DirectX::XMVector3Length(closest_normal)) < 0.0001f)
		{
			continue;
		}

        // 슬라이딩 벡터 계산
        DirectX::XMVECTOR normalized_closest_normal = DirectX::XMVector3Normalize(closest_normal);

        // 충돌후 관통된 깊이를 구해 깊이만큼 벡터로 튀어나오게 계산        
        // 충돌 깊이 보정 벡터 계산
        float depth = CalculatePenetrationDepth(object.second, normalized_closest_normal);
        DirectX::XMVECTOR depth_delta = DirectX::XMVectorZero();
        if (depth > 0.00001f)
        {
            // 깊이 벡터 계산
            depth_delta = DirectX::XMVectorScale(normalized_closest_normal, depth);
        }

        // Y축(윗면) 충돌시 애니메이션 스테이트 설정
        if (DirectX::XMVectorGetY(normalized_closest_normal) > 0.9f)
        {
            // 점프 시작시에는 적용 x
            if (player->obj_state_ == Object_State::STATE_JUMP_START)
            {
                continue;
            }

            player->on_ground_ = true;
            slide_vector = DirectX::XMVectorSetY(slide_vector, 0.0f);

            // y축에서 공중에 뜨는것 방지 살짝 눌러줌
            depth_delta = DirectX::XMVectorAdd(depth_delta, DirectX::XMVectorNegate(normalized_closest_normal));

            // 점프 중 땅에 닿으면 점프 종료로 변환
            if (player->obj_state_ == Object_State::STATE_JUMP_IDLE)
            {
                player->obj_state_ = Object_State::STATE_JUMP_END;
                player->moveable_ = false;
                player->stop_skill_time_ = JUMP_END_TIME;
            }
        }
        else
        {
            // 벽에 끼지 않게 살짝 튕김
            depth_delta = DirectX::XMVectorAdd(depth_delta, normalized_closest_normal);
            // velocity를 법선에 투영해 슬라이딩 벡터 계산
            // S = V - (V . N) * N
            DirectX::XMVECTOR P = DirectX::XMVectorScale(normalized_closest_normal,
                DirectX::XMVectorGetX(
                    DirectX::XMVector3Dot(slide_vector, normalized_closest_normal)));
            slide_vector = DirectX::XMVectorSubtract(slide_vector, P);
        }

        // 깊이만큼 이동
        DirectX::XMStoreFloat3(&player->depth_delta_, depth_delta);
        player->position_ = MathHelper::Add(player->position_, player->depth_delta_);
        player->depth_delta_ = DirectX::XMFLOAT3();
        UpdateOBB(player);
    }

    DirectX::XMStoreFloat3(&player->velocity_vector_, slide_vector);
}

void CatPlayer::CheckCheeseIntersects(Player* player, float deltaTime)
{
}


float CatPlayer::CalculatePenetrationDepth(const ObjectOBB& obj, DirectX::XMVECTOR normal)
{
    // 1. obb의 중심점끼리의 거리를 부딪힌 면의 노멀벡터에 투영하면 두 obb 사이의 현재 거리를 알 수 있다.
    // 2. 플레이어의 obb 축을 다 투영해 더하고, 부딪힌 물체의 중심에서부터 부딪힌 면의 extend 길이를 합하면 관통을 안했을때의 길이를 알수 있다.
    // 3. 2 - 1을 해서 0.0f보다 크면 관통된 길이값을 구할 수 있다.

    DirectX::XMVECTOR player_center = DirectX::XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR obj_center = DirectX::XMLoadFloat3(&obj.obb.Center);
    
    // 두 obb의 중심점 사이의 거리
    DirectX::XMVECTOR center_distance = DirectX::XMVectorSubtract(player_center, obj_center);
    float curr_distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(center_distance, normal));

    // 플레이어의 obb 축을 다 투영해 더함
    float player_axis_proj_distance = CalculateOBBAxisProj(normal, obb_);
    // 부딪힌 물체의 중심에서부터 부딪힌 면의 extend 길이를 합함
    float obj_axis_proj_distance =
        fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(obj.worldAxes[0], normal))) * obj.obb.Extents.x +
        fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(obj.worldAxes[1], normal))) * obj.obb.Extents.y +
        fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(obj.worldAxes[2], normal))) * obj.obb.Extents.z;

    // 2 - 1 관통된 길이값
    float penetration_depth = std::fabs(obj_axis_proj_distance + player_axis_proj_distance) - std::fabs(curr_distance);

    // 깊이가 0 이하면 관통하지 않음
    return penetration_depth > 0.0f ? penetration_depth : 0.0f;
}

float CatPlayer::CalculateOBBAxisProj(DirectX::XMVECTOR axis, const DirectX::BoundingOrientedBox& obb)
{
	// OBB의 축(로컬 X, Y, Z축)
	DirectX::XMVECTOR localAxes[3] = {
		DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),  // 로컬 X축
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),  // 로컬 Y축
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)   // 로컬 Z축
	};

	// 쿼터니언을 이용해 로컬 축을 회전해 투영할 축을 구해줌
	DirectX::XMVECTOR quaternion = DirectX::XMLoadFloat4(&obb.Orientation);
	DirectX::XMVECTOR worldAxes[3];
	for (int i = 0; i < 3; ++i)
	{
		worldAxes[i] = DirectX::XMVector3Rotate(localAxes[i], quaternion);
	}

	// 각 OBB의 축을 충돌면 법선에 투영한 값에 Extents를 곱해 합산해서 길이 구함
	float radius =
		fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(worldAxes[0], axis))) * obb.Extents.x +
		fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(worldAxes[1], axis))) * obb.Extents.y +
		fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(worldAxes[2], axis))) * obb.Extents.z;

	return radius;
}

bool CatPlayer::CalculatePhysics(Player* player, float deltaTime)
{
    bool need_update = false;

	// 각도 회전 체크
	if (player->prev_player_pitch_ != player->player_pitch_)
	{
        need_update = true;
		player->prev_player_pitch_ = player->player_pitch_;
	}

	player->delta_position_ = DirectX::XMFLOAT3();

	// 속도 계산 및 이동 여부 체크
	if (player->UpdateVelocity(deltaTime))
	{
        need_update = true;
	}
        
    // 정지시 감속
    player->ApplyDecelerationIfStop(deltaTime);
    // 힘 적용
	player->ApplyForces(deltaTime);
    // 마찰력 적용
	player->ApplyFriction(deltaTime);
	// 위치 업데이트
	player->position_ = MathHelper::Add(player->position_, player->delta_position_);

	//std::cout << "현재 위치 : " << player->position_.x << ", " << player->position_.y << ", " << player->position_.z << std::endl;

    // 스킬 시간이 다 갈때까지 state 유지
    if (player->stop_skill_time_ > 0.001f)
    {
        player->stop_skill_time_ -= deltaTime;
        need_update = true;
    }
    // 움직이지 못하고, 스킬 시전중에만
    else if(player->stop_skill_time_ < 0.001f && player->moveable_ == false)
    {
        // 공격이 끝났다면 공격박스 초기화
        if (player->obj_state_ == Object_State::STATE_ACTION_ONE)
        {
            InitAttackOBB(player, g_sessions[player->comp_key_.session_id].cat_attack_obb_);
        }

        player->stop_skill_time_ = 0.0f;
        player->moveable_ = true;
        player->obj_state_ = Object_State::STATE_IDLE;
        need_update = true;
    }

    // 땅에 붙어있을때 && 점프끝 && Action 이 아닐때, && moveable == true 속도에따라 걷거나 멈춤
    else if (player->stop_skill_time_ < 0.001f && player->moveable_ == true)
    {
        if (player->on_ground_ == true)
        {
            if (player->speed_ > 0.05f && player->obj_state_ == Object_State::STATE_IDLE)
            {
                player->obj_state_ = Object_State::STATE_MOVE;
                need_update = true;
            }
            else if (player->speed_ <= 0.05f && player->obj_state_ == Object_State::STATE_MOVE)
            {
                player->obj_state_ = Object_State::STATE_IDLE;
                need_update = true;
            }
        }
    }
        
	return need_update;
}

void CatPlayer::UpdateOBB(Player* player)
{
	// OBB 업데이트
	obb_.Center = player->position_;
    // pivot이 아래 기준이라 조정
    obb_.Center.y += obb_.Extents.y;
	// OBB의 회전 값 갱신
	obb_.Orientation = player->rotation_quat_;

    // OBB의 중심점을 기준으로 한 BoundingSphere 업데이트
    player_sphere_.Center = obb_.Center;
}

void CatPlayer::ActionOne(Player* player)
{
    if(player->moveable_ == false)
	{
		return;
	}
    std::cout << "공격****" << std::endl;
    player->moveable_ = false;
    player->stop_skill_time_ = CAT_ATTACK_TIME;
    player->obj_state_ = Object_State::STATE_ACTION_ONE;
    
    int session_id = player->comp_key_.session_id;

    // 박스 생성
    CreateAttackOBB(player, g_sessions[session_id].cat_attack_obb_);
    g_sessions[session_id].cat_attack_ = true;
    
    // 해당 공격에 맞은 쥐
    g_sessions[session_id].CheckAttackedMice();
}


void CatPlayer::CreateAttackOBB(Player* player, DirectX::BoundingOrientedBox& box)
{
    // 캐릭터의 위치 및 방향 벡터
    DirectX::XMVECTOR player_position = XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR look = DirectX::XMVector3Normalize(XMLoadFloat3(&player->look_));

    g_sessions[player->comp_key_.session_id].cat_attack_direction_ = player->look_;

    // 공격 OBB의 중심점 설정
    // 캐릭터 앞 공격 범위의 절반만큼 이동
    DirectX::XMVECTOR center_offset = DirectX::XMVectorScale(look, attack_range);
    DirectX::XMVECTOR attack_center = DirectX::XMVectorAdd(player_position, center_offset);
    DirectX::XMStoreFloat3(&box.Center, attack_center);

    // 고양이가 길어서 z축은 절반만 사용
    DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(attack_width * 2.0f, attack_height * 2.0f, attack_range);
    box.Extents = extents;

    // 쿼터니언은 그대로 사용
    box.Orientation = obb_.Orientation;
}

void CatPlayer::InitAttackOBB(Player* player, DirectX::BoundingOrientedBox& box)
{
    // 공격 OBB 초기화
	box.Center = DirectX::XMFLOAT3(0.0f, -9999.0f, 0.0f);
	box.Extents = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	box.Orientation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    // 해당 공격에 맞은 쥐들 초기화
    for (auto& mouse_attched : g_sessions[player->comp_key_.session_id].cat_attacked_player_)
    {
        mouse_attched.second = false;
    }
}
