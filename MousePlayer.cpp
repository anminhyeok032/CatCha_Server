#include "MousePlayer.h"
#include "Player.h"
#include "MapData.h"

void MousePlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard ������Ʈ
	player->SetKeyState(action, is_key_pressed);

	// ���� ������Ʈ ��û
	player->RequestSessionUpdate();
}

void MousePlayer::CheckIntersects(Player* player, float deltaTime)
{
	player->ApplyGravity(FIXED_TIME_STEP);
	// 1. ���� velocity �� �̿��Ͽ� ���� ��ġ ���
	// 2. ���� ��ġ�� ���� OBB �浹 �˻�
	// 3. �浹 ��, �浹�� ��ü�� ���� �̿��� �ﰢ�� ����� �ﰢ���� velocity vector�� ���� �˻� �� ���� �˻�
	// 4. ã�� �ﰢ���� ��ֺ��͸� �̿��ؼ� velocity �����ؼ� ĳ������ velocity ���͸� ����

	// ���� ��ġ�� �ӵ��� �������� ���� ��ġ ���
	DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&player->position_);
	DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&player->velocity_vector_);

	float velocity_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity));
	// �ӵ��� 0�̸� �浹üũ x
	if (velocity_length < 0.0001f)
	{
		return;
	}

	// ���� ��ġ ������Ʈ 
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(currentPos, DirectX::XMVectorScale(velocity, deltaTime));
	DirectX::XMFLOAT3 newPosFloat3; // �� ��ġ�� ���� OBB ������ ���� XMFLOAT3 �� ��ȯ
	DirectX::XMStoreFloat3(&newPosFloat3, newPos);

	// ���� ��ġ OBB ����
	DirectX::BoundingOrientedBox newPositionOBB{ newPosFloat3, obb_.Extents, obb_.Orientation };

	DirectX::XMVECTOR slide_vector = velocity;

	// ��� ������Ʈ �浹üũ
	for (const auto& object : g_obbData)
	{
		// �ӵ��� 0�̸� �浹üũ x
		if (DirectX::XMVectorGetX(DirectX::XMVector3Length(slide_vector)) < 0.0001f)
		{
			break;
		}
		// �ش� ��ü�� ������ġ OBB�� �浹 �˻�
		if (false == object.second.obb.Intersects(newPositionOBB))
		{
			continue;
		}
		std::cout << "�浹�� : " << object.first << std::endl;

		float min_distance = FLT_MAX;								// ���� �Ÿ��� ª�� �浹 ���������� �Ÿ�
		DirectX::XMVECTOR closest_normal = DirectX::XMVectorZero();	// ���� ����� �� normal ����

		// velocity normalȭ
		DirectX::XMVECTOR normalized_velocity = DirectX::XMVector3Normalize(slide_vector);

		// OBB ������ �迭
		DirectX::XMFLOAT3 corners[8];
		object.second.obb.GetCorners(corners);

		// OBB�� �����ϴ� �� ��(�ﰢ������ ������ ��� 2��)�� ���� �浹 �˻�
		for (const auto& indices : g_triangle_indices)
		{
			// �ﰢ���� �����ϴ� �� ���� �ε�
			DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&corners[indices[0]]);
			DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&corners[indices[1]]);
			DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&corners[indices[2]]);

			// �ﰢ���� velocity ������ �浹 �˻�
			// t : �浹 ���������� �Ÿ�
			float t;
			// �ﰢ���� velocity ���Ͱ� �����ϴ��� Ȯ��
			bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_velocity, p1, p2, p3, t);

			// ���� && ���� ����� �Ÿ����� Ȯ��
			if (true == intersects && t < min_distance)
			{
				min_distance = t;
				// ���� ����� ���� normal ���� ���
				closest_normal = DirectX::XMVector3Normalize(
					DirectX::XMVector3Cross(
						DirectX::XMVectorSubtract(p2, p1), DirectX::XMVectorSubtract(p3, p1)
					)
				);
			}
		}

		// �����̵� ���� ���
		DirectX::XMVECTOR normalized_closest_normal = DirectX::XMVector3Normalize(closest_normal);

		// velocity�� ������ ������ �����̵� ���� ���
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

	// ���� ȸ�� üũ
	if (player->prev_player_pitch_ != player->player_pitch_)
	{
		is_moving = true;
		player->prev_player_pitch_ = player->player_pitch_;
	}

	float time_remaining = (deltaTime < 1.0f) ? deltaTime : 1.0f;	// �ִ� 1�ʱ����� ���
	const int MAX_ITERATIONS = 100;									// ���� ���� ����
	int iterations = 0;												// ���� üũ

	while (time_remaining >= FIXED_TIME_STEP) 
	{
		player->delta_position_ = DirectX::XMFLOAT3();

		// �ӵ� ��� �� �̵� ���� üũ
		if (player->UpdateVelocity(FIXED_TIME_STEP))
		{
			is_moving = true;
		}

		// ��Ÿ ���� ó��
		player->ApplyDecelerationIfStop(FIXED_TIME_STEP);
		player->ApplyForces(FIXED_TIME_STEP);
		player->ApplyFriction(FIXED_TIME_STEP);

		// ��ġ ������Ʈ
		player->position_ = MathHelper::Add(player->position_, player->delta_position_);
		//std::cout << "CalculatePhysics : ���� �ӵ�: " << player->velocity_vector_.x << ", " << player->velocity_vector_.y << ", " << player->velocity_vector_.z << std::endl;
		//std::cout << "���� ��ġ : " << player->position_.x << ", " << player->position_.y << ", " << player->position_.z << std::endl;

		// ���� �ð� ���ܸ�ŭ ����
		time_remaining -= FIXED_TIME_STEP;

		iterations++;
	}

	return is_moving;
}


void MousePlayer::UpdateOBB(Player* player)
{
	// OBB ������Ʈ
	obb_.Center = player->position_;
	// OBB�� ȸ�� �� ����
	obb_.Orientation = player->rotation_quat_;
}