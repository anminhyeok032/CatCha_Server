#include "MousePlayer.h"
#include "Player.h"
#include "MapData.h"

void MousePlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard 업데이트
	player->SetKeyState(action, is_key_pressed);

	// 세션 업데이트 요청
	player->RequestSessionUpdate();
}

void MousePlayer::CheckIntersects(Player* player, float deltaTime)
{
	player->ApplyGravity(FIXED_TIME_STEP);
	// 1. 현재 velocity 를 이용하여 예상 위치 계산
	// 2. 예상 위치에 대한 OBB 충돌 검사
	// 3. 충돌 시, 충돌된 물체의 점을 이용해 삼각형 만들고 삼각형과 velocity vector의 교차 검사 및 깊이 검사
	// 4. 찾은 삼각형의 노멀벡터를 이용해서 velocity 투영해서 캐릭터의 velocity 벡터를 조정

	// 현재 위치와 속도를 바탕으로 예상 위치 계산
	DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&player->position_);
	DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&player->velocity_vector_);

	float velocity_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity));
	// 속도가 0이면 충돌체크 x
	if (velocity_length < 0.0001f)
	{
		return;
	}

	// 예상 위치 업데이트 
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(currentPos, DirectX::XMVectorScale(velocity, deltaTime));
	DirectX::XMFLOAT3 newPosFloat3; // 새 위치에 대한 OBB 생성을 위해 XMFLOAT3 로 변환
	DirectX::XMStoreFloat3(&newPosFloat3, newPos);

	// 예상 위치 OBB 생성
	DirectX::BoundingOrientedBox newPositionOBB{ newPosFloat3, obb_.Extents, obb_.Orientation };

	DirectX::XMVECTOR slide_vector = velocity;

	// 모든 오브젝트 충돌체크
	for (const auto& object : g_obbData)
	{
		// 속도가 0이면 충돌체크 x
		if (DirectX::XMVectorGetX(DirectX::XMVector3Length(slide_vector)) < 0.0001f)
		{
			break;
		}
		// 해당 물체의 예상위치 OBB와 충돌 검사
		if (false == object.second.obb.Intersects(newPositionOBB))
		{
			continue;
		}
		std::cout << "충돌함 : " << object.first << std::endl;

		float min_distance = FLT_MAX;								// 가장 거리가 짧은 충돌 지점까지의 거리
		DirectX::XMVECTOR closest_normal = DirectX::XMVectorZero();	// 가장 가까운 면 normal 벡터

		// velocity normal화
		DirectX::XMVECTOR normalized_velocity = DirectX::XMVector3Normalize(slide_vector);

		// OBB 꼭짓점 배열
		DirectX::XMFLOAT3 corners[8];
		object.second.obb.GetCorners(corners);

		// OBB를 구성하는 각 면(삼각형으로 나눠서 면당 2번)에 대해 충돌 검사
		for (const auto& indices : g_triangle_indices)
		{
			// 삼각형을 구성하는 각 점을 로드
			DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&corners[indices[0]]);
			DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&corners[indices[1]]);
			DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&corners[indices[2]]);

			// 삼각형과 velocity 벡터의 충돌 검사
			// t : 충돌 지점까지의 거리
			float t;
			// 삼각형과 velocity 벡터가 교차하는지 확인
			bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_velocity, p1, p2, p3, t);

			// 교차 && 가장 가까운 거리인지 확인
			if (true == intersects && t < min_distance)
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

		// 슬라이딩 벡터 계산
		DirectX::XMVECTOR normalized_closest_normal = DirectX::XMVector3Normalize(closest_normal);

		// velocity를 법선에 투영해 슬라이딩 벡터 계산
		// S = V - (V . N) * N
		DirectX::XMVECTOR P = DirectX::XMVectorScale(closest_normal,
			DirectX::XMVectorGetX(
				DirectX::XMVector3Dot(slide_vector, normalized_closest_normal)));
		slide_vector = DirectX::XMVectorSubtract(slide_vector, P);
	}
	DirectX::XMStoreFloat3(&player->velocity_vector_, slide_vector);
	//std::cout << "velocity : " << player->velocity_vector_.x << ", " << player->velocity_vector_.y << ", " << player->velocity_vector_.z << std::endl;
}



bool MousePlayer::CalculatePhysics(Player* player, float deltaTime)
{
	bool is_moving = false;

	// 각도 회전 체크
	if (player->prev_player_pitch_ != player->player_pitch_)
	{
		is_moving = true;
		player->prev_player_pitch_ = player->player_pitch_;
	}

	float time_remaining = (deltaTime < 1.0f) ? deltaTime : 1.0f;	// 최대 1초까지만 계산
	const int MAX_ITERATIONS = 100;									// 무한 루프 방지
	int iterations = 0;												// 루프 체크

	while (time_remaining >= FIXED_TIME_STEP) 
	{
		player->delta_position_ = DirectX::XMFLOAT3();

		// 속도 계산 및 이동 여부 체크
		if (player->UpdateVelocity(FIXED_TIME_STEP))
		{
			is_moving = true;
		}

		// 기타 물리 처리
		player->ApplyDecelerationIfStop(FIXED_TIME_STEP);
		player->ApplyForces(FIXED_TIME_STEP);
		player->ApplyFriction(FIXED_TIME_STEP);

		// 위치 업데이트
		player->position_ = MathHelper::Add(player->position_, player->delta_position_);
		//std::cout << "CalculatePhysics : 최종 속도: " << player->velocity_vector_.x << ", " << player->velocity_vector_.y << ", " << player->velocity_vector_.z << std::endl;
		//std::cout << "현재 위치 : " << player->position_.x << ", " << player->position_.y << ", " << player->position_.z << std::endl;

		// 고정 시간 스텝만큼 감소
		time_remaining -= FIXED_TIME_STEP;

		iterations++;
	}

	return is_moving;
}


void MousePlayer::UpdateOBB(Player* player)
{
	// OBB 업데이트
	obb_.Center = player->position_;
	// OBB의 회전 값 갱신
	obb_.Orientation = player->rotation_quat_;
}