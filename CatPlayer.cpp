#include "CatPlayer.h"
#include "Player.h"
#include "MapData.h"

void CatPlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

    // ��ų �������� �ƴ� ��쿡�� ��ų Ű �̺�Ʈ ó��
    if (player->moveable_ == true)
    {
        switch (action)
        {
        case Action::ACTION_ONE:
            //ActionOne(player);
            break;
        default:
            break;
        }
    }

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

	// keyboard ������Ʈ
	player->SetKeyState(action, is_key_pressed);

	// ���� ������Ʈ ��û
	player->RequestUpdate();
}

void CatPlayer::Jump(Player* player)
{
    if (player->obj_state_ == Object_State::STATE_IDLE || player->obj_state_ == Object_State::STATE_MOVE)
    {
        //std::cout << "����!!!" << std::endl;
        // ���� �������� ����
        player->obj_state_ = Object_State::STATE_JUMP_START;
        // ���� �Ŀ��� ����
        player->velocity_vector_.y = player->jump_power_;
        // ������ �ѹ��� ����ǰ� Ű ��ǲ map���� ����
        player->keyboard_input_[Action::ACTION_JUMP] = false;
        // ������ ������ �������ɷ� ����
        player->on_ground_ = false;
        // �ִϸ��̼� �ð� ����
        player->stop_skill_time_ = 0.666666687f;
    }
    else
    {
        // ������ �ѹ��� ����ǰ� Ű ��ǲ map���� ����
        player->keyboard_input_[Action::ACTION_JUMP] = false;
    }
}


void CatPlayer::CheckIntersects(Player* player, float deltaTime)
{
    // 1. bounding sphere�� �̿��� �˻� ���� ���
    // 2. ���� ��ġ�� ���� OBB �浹 �˻� (xz��, y�� �и� �˻�)
    // 3. �浹 ��, �浹�� ��ü�� ���� �̿��� �ﰢ�� ����� �ﰢ���� �� ���������� ������ ���� �˻� �� ���� �˻�
    // 4. ã�� �ﰢ���� ��ֺ��͸� �̿��ؼ� velocity �����ؼ� ĳ������ velocity ���͸� ����
    // 5. ���� ���̰��� �̿��� ��ü�� ���� �ʵ��� ��ġ ����

    player->ApplyGravity(deltaTime);

    DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR slide_vector = DirectX::XMLoadFloat3(&player->velocity_vector_);


    // ��� ������Ʈ �浹üũ
    for (const auto& object : g_obbData)
    {
        // slidevector�� 0�̸� �浹üũ ���ʿ�
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(slide_vector)) < 0.0001f)
        {
            break;
        }

        // �浹 üũ Ƚ���� ���̱� ���� BoundingSphere�� ���� üũ
        if (false == player_sphere_.Intersects(object.second.obb))
        {
            continue;
        }

        // ���� �Ÿ��� ª�� �浹 ���������� �Ÿ�
        float depth_ = FLT_MIN;
        // ���� ����� �� normal ����
        DirectX::XMVECTOR closest_normal = DirectX::XMVectorZero();
       
        if(false == object.second.obb.Intersects(obb_))
        {
			continue;
		}

        //std::cout << "�浹�� : " << object.first << std::endl;

        //���� �Ÿ��� ª�� �浹 ���������� �Ÿ�
        float min_distance = FLT_MAX;


        // �ε��� ��ü OBB ������ �迭
        DirectX::XMFLOAT3 corners[8];
        object.second.obb.GetCorners(corners);

        // **1. ��/�Ʒ� �� �˻�**
        // velocity�� y�ุ ���
        DirectX::XMVECTOR d = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&object.second.obb.Center), currentPos);
        DirectX::XMVECTOR check_d = DirectX::XMVectorSet(0.0f, DirectX::XMVectorGetY(d), 0.0f, 0.0f);
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(check_d)) > 0.0001f)
        {
            DirectX::XMVECTOR normalized_d = DirectX::XMVector3Normalize(check_d);
            // OBB�� �����ϴ� �� ��(�ﰢ������ ������ ��� 2��)�� ���� �浹 �˻�
            for (int i = 0; i < 4; ++i) // ��(4~7)�� �Ʒ�(0~3) �鸸 �˻�
            {
                // �ﰢ���� �����ϴ� �� ���� �ε�
                const auto& indices = g_triangle_indices[i];
                DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&corners[indices[0]]);
                DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&corners[indices[1]]);
                DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&corners[indices[2]]);

                // �ﰢ���� velocity�� ���� ���� Ȯ��
                float t;    // �浹 ���������� �Ÿ�
                // �ﰢ���� velocity ���Ͱ� �����ϴ��� Ȯ��
                bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_d, p1, p2, p3, t);

                // ���� && ���� ����� �Ÿ����� Ȯ��
                if (intersects && t < min_distance)
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
        }

        // **2. ���� �˻� (��, ��, ��, ��)**
        // velocity�� xz�� ���
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

                // �ﰢ���� velocity�� ���� ���� Ȯ��
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
        
        // ��ġ�� ��� �浹�� �ȵ� ������ �Ǿ����
        if(DirectX::XMVectorGetX(DirectX::XMVector3Length(closest_normal)) < 0.0001f)
		{
			continue;
		}

        // �����̵� ���� ���
        DirectX::XMVECTOR normalized_closest_normal = DirectX::XMVector3Normalize(closest_normal);

        // �浹�� ����� ���̸� ���� ���̸�ŭ ���ͷ� Ƣ����� ���        
        // �浹 ���� ���� ���� ���
        float depth = CalculatePenetrationDepth(object.second, normalized_closest_normal);
        DirectX::XMVECTOR depth_delta = DirectX::XMVectorZero();
        if (depth > 0.00001f)
        {
            // ���� ���� ���
            depth_delta = DirectX::XMVectorScale(normalized_closest_normal, depth);
        }

        // Y��(����) �浹�� �ִϸ��̼� ������Ʈ ����
        if (DirectX::XMVectorGetY(normalized_closest_normal) > 0.9f)
        {
            // ���� ���۽ÿ��� ���� x
            if (DirectX::XMVectorGetY(slide_vector) > 0.0f)
            {
                continue;
            }

            player->on_ground_ = true;
            slide_vector = DirectX::XMVectorSetY(slide_vector, 0.0f);

            // y�࿡�� ���߿� �ߴ°� ���� ��¦ ������
            depth_delta = DirectX::XMVectorAdd(depth_delta, DirectX::XMVectorNegate(normalized_closest_normal));

            // ���� �� ���� ������ ���� ����� ��ȯ
            if (player->obj_state_ == Object_State::STATE_JUMP_IDLE)
            {
                player->obj_state_ = Object_State::STATE_JUMP_END;
                player->moveable_ = false;
                player->stop_skill_time_ = JUMP_END_TIME;
            }
        }
        else
        {
            // ���� ���� �ʰ� ��¦ ƨ��
            depth_delta = DirectX::XMVectorAdd(depth_delta, normalized_closest_normal);
            // velocity�� ������ ������ �����̵� ���� ���
            // S = V - (V . N) * N
            DirectX::XMVECTOR P = DirectX::XMVectorScale(normalized_closest_normal,
                DirectX::XMVectorGetX(
                    DirectX::XMVector3Dot(slide_vector, normalized_closest_normal)));
            slide_vector = DirectX::XMVectorSubtract(slide_vector, P);
        }

        // ���̸�ŭ �̵�
        DirectX::XMStoreFloat3(&player->depth_delta_, depth_delta);
        player->position_ = MathHelper::Add(player->position_, player->depth_delta_);
        player->depth_delta_ = DirectX::XMFLOAT3();
        UpdateOBB(player);
    }

    DirectX::XMStoreFloat3(&player->velocity_vector_, slide_vector);
}

bool CatPlayer::CheckCheeseIntersects(Player* player, float deltaTime)
{
    bool crashing_cheese = false;

    DirectX::BoundingBox ChesseAABB;
    DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR slide_vector = DirectX::XMLoadFloat3(&player->velocity_vector_);

    // ���� OBB
    DirectX::XMVECTOR pred_pos = DirectX::XMVectorAdd(currentPos, DirectX::XMVectorScale(slide_vector, deltaTime));
    DirectX::XMFLOAT3 pred_pos_f3;
    DirectX::XMStoreFloat3(&pred_pos_f3, pred_pos);
    DirectX::BoundingSphere player_sphere = DirectX::BoundingSphere(pred_pos_f3, obb_.Extents.y);

    int session_id = player->comp_key_.session_id;

    for (const auto& cheese : g_sessions[session_id].cheese_octree_)
    {
        // ū ������ �˻� ���� ���̱�
        if (false == player_sphere_.Intersects(cheese.boundingBox))
        {
            continue;
        }

        // �浹�� AABB �޾Ƽ� AABB ������ velocity �����̵� ��Ű��
        crashing_cheese = cheese.IntersectCheck(player_sphere, ChesseAABB);

        // �����س� AABB�� �̿��� �����̵� ���� ���
        if (true == crashing_cheese)
        {
            // AABB �߽ɰ� Extent �ε�
            DirectX::XMVECTOR aabbCenter = DirectX::XMLoadFloat3(&ChesseAABB.Center);
            DirectX::XMVECTOR aabbExtents = DirectX::XMLoadFloat3(&ChesseAABB.Extents);

            DirectX::XMVECTOR collisionCenter = DirectX::XMLoadFloat3(&obb_.Center);

            // ���� ���� ���� �� �� �ະ �Ÿ� ���
            DirectX::XMVECTOR centerVector = DirectX::XMVectorSubtract(collisionCenter, aabbCenter);
            DirectX::XMVECTOR absCenterVector = DirectX::XMVectorAbs(centerVector);

            // �� �ະ ��� �Ÿ� (�Ÿ� / Extent) ���
            DirectX::XMVECTOR relativeDistances = DirectX::XMVectorDivide(absCenterVector, aabbExtents);

            // ���� ū ���� �ε����� ã��
            // �ʱⰪ: X��
            DirectX::XMVECTOR maxVector = DirectX::XMVectorSplatX(relativeDistances);
            int maxAxis = 0;

            // y�� �� ũ�� Y������ ����
            if (DirectX::XMVectorGetY(relativeDistances) > DirectX::XMVectorGetX(maxVector))
            {
                maxVector = DirectX::XMVectorSplatY(relativeDistances);
                maxAxis = 1;
            }
            // z�� �� ũ�� Z������ ����
            if (DirectX::XMVectorGetZ(relativeDistances) > DirectX::XMVectorGetX(maxVector))
            {
                maxAxis = 2;
            }

            // �浹 ���� ���� ���� ����
            // �߽����� ������ ��ȣ�� ����
            float x_set = (DirectX::XMVectorGetX(centerVector) >= 0.0f) ? 1.0f : -1.0f;
            float y_set = (DirectX::XMVectorGetY(centerVector) >= 0.0f) ? 1.0f : -1.0f;
            float z_set = (DirectX::XMVectorGetZ(centerVector) >= 0.0f) ? 1.0f : -1.0f;

            DirectX::XMVECTOR normal = DirectX::XMVectorZero();
            switch (maxAxis)
            {
                // +X
            case 0:
                normal = DirectX::XMVectorSet(x_set, 0.0f, 0.0f, 0.0f);
                break;
                // +Y
            case 1:
                normal = DirectX::XMVectorSet(0.0f, y_set, 0.0f, 0.0f);
                break;
                // +Z
            case 2:
                normal = DirectX::XMVectorSet(0.0f, 0.0f, z_set, 0.0f);
                break;
            }

            // Y��(����) �浹�� �ִϸ��̼� ������Ʈ ����
            if (DirectX::XMVectorGetY(normal) > 0.9f)
            {
                // ���� ���۽ÿ��� ���� x
                if (player->obj_state_ == Object_State::STATE_JUMP_START)
                {
                    continue;
                }

                player->on_ground_ = true;
                slide_vector = DirectX::XMVectorSetY(slide_vector, 0.0f);

                // ���� �� ���� ������ ���� ����� ��ȯ
                if (player->obj_state_ == Object_State::STATE_JUMP_IDLE)
                {
                    player->obj_state_ = Object_State::STATE_JUMP_END;
                }
            }
            else
            {
                // velocity�� ������ ������ �����̵� ���� ���
                // S = V - (V . N) * N
                DirectX::XMVECTOR P = DirectX::XMVectorScale(normal,
                    DirectX::XMVectorGetX(
                        DirectX::XMVector3Dot(slide_vector, normal)));
                slide_vector = DirectX::XMVectorSubtract(slide_vector, P);
            }
        }
    }
    if( true == crashing_cheese )
	{
		DirectX::XMStoreFloat3(&player->velocity_vector_, slide_vector);

        return true;
	}
    return false;
}


float CatPlayer::CalculatePenetrationDepth(const ObjectOBB& obj, DirectX::XMVECTOR normal)
{
    // 1. obb�� �߽��������� �Ÿ��� �ε��� ���� ��ֺ��Ϳ� �����ϸ� �� obb ������ ���� �Ÿ��� �� �� �ִ�.
    // 2. �÷��̾��� obb ���� �� ������ ���ϰ�, �ε��� ��ü�� �߽ɿ������� �ε��� ���� extend ���̸� ���ϸ� ������ ���������� ���̸� �˼� �ִ�.
    // 3. 2 - 1�� �ؼ� 0.0f���� ũ�� ����� ���̰��� ���� �� �ִ�.

    DirectX::XMVECTOR player_center = DirectX::XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR obj_center = DirectX::XMLoadFloat3(&obj.obb.Center);
    
    // �� obb�� �߽��� ������ �Ÿ�
    DirectX::XMVECTOR center_distance = DirectX::XMVectorSubtract(player_center, obj_center);
    float curr_distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(center_distance, normal));

    // �÷��̾��� obb ���� �� ������ ����
    float player_axis_proj_distance = CalculateOBBAxisProj(normal, obb_);
    // �ε��� ��ü�� �߽ɿ������� �ε��� ���� extend ���̸� ����
    float obj_axis_proj_distance =
        fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(obj.worldAxes[0], normal))) * obj.obb.Extents.x +
        fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(obj.worldAxes[1], normal))) * obj.obb.Extents.y +
        fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(obj.worldAxes[2], normal))) * obj.obb.Extents.z;

    // 2 - 1 ����� ���̰�
    float penetration_depth = std::fabs(obj_axis_proj_distance + player_axis_proj_distance) - std::fabs(curr_distance);

    // ���̰� 0 ���ϸ� �������� ����
    return penetration_depth > 0.0f ? penetration_depth : 0.0f;
}

float CatPlayer::CalculateOBBAxisProj(DirectX::XMVECTOR axis, const DirectX::BoundingOrientedBox& obb)
{
	// OBB�� ��(���� X, Y, Z��)
	DirectX::XMVECTOR localAxes[3] = {
		DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),  // ���� X��
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),  // ���� Y��
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)   // ���� Z��
	};

	// ���ʹϾ��� �̿��� ���� ���� ȸ���� ������ ���� ������
	DirectX::XMVECTOR quaternion = DirectX::XMLoadFloat4(&obb.Orientation);
	DirectX::XMVECTOR worldAxes[3];
	for (int i = 0; i < 3; ++i)
	{
		worldAxes[i] = DirectX::XMVector3Rotate(localAxes[i], quaternion);
	}

	// �� OBB�� ���� �浹�� ������ ������ ���� Extents�� ���� �ջ��ؼ� ���� ����
	float radius =
		fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(worldAxes[0], axis))) * obb.Extents.x +
		fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(worldAxes[1], axis))) * obb.Extents.y +
		fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(worldAxes[2], axis))) * obb.Extents.z;

	return radius;
}

bool CatPlayer::CalculatePhysics(Player* player, float deltaTime)
{
    bool need_update = false;

	// ���� ȸ�� üũ
	if (player->prev_player_pitch_ != player->player_pitch_)
	{
        need_update = true;
		player->prev_player_pitch_ = player->player_pitch_;
	}

	player->delta_position_ = DirectX::XMFLOAT3();

	// �ӵ� ��� �� �̵� ���� üũ
	if (player->UpdateVelocity(deltaTime))
	{
        need_update = true;
	}
        
    // ������ ����
    player->ApplyDecelerationIfStop(deltaTime);
    // �� ����
	player->ApplyForces(deltaTime);
    // ������ ����
	player->ApplyFriction(deltaTime);
	// ��ġ ������Ʈ
	player->position_ = MathHelper::Add(player->position_, player->delta_position_);

	//std::cout << "���� ��ġ : " << player->position_.x << ", " << player->position_.y << ", " << player->position_.z << std::endl;

    // ��ų �ð��� �� �������� state ����
    if (player->stop_skill_time_ > 0.001f)
    {
        player->stop_skill_time_ -= deltaTime;
        need_update = true;
    }
    // �������� ���ϰ�, ��ų �����߿���
    else if(player->stop_skill_time_ < 0.001f && player->moveable_ == false)
    {
        // ������ �����ٸ� ���ݹڽ� �ʱ�ȭ
        if (player->obj_state_ == Object_State::STATE_ACTION_ONE)
        {
            InitAttackOBB(player, g_sessions[player->comp_key_.session_id].cat_attack_obb_);
        }

        player->stop_skill_time_ = 0.0f;
        player->moveable_ = true;
        player->obj_state_ = Object_State::STATE_IDLE;
        need_update = true;
    }

    // ���� �پ������� && ������ && Action �� �ƴҶ�, && moveable == true �ӵ������� �Ȱų� ����
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
	// OBB ������Ʈ
	obb_.Center = player->position_;
    // pivot�� �Ʒ� �����̶� ����
    obb_.Center.y += obb_.Extents.y;
	// OBB�� ȸ�� �� ����
	obb_.Orientation = player->rotation_quat_;

    // OBB�� �߽����� �������� �� BoundingSphere ������Ʈ
    player_sphere_.Center = obb_.Center;
}

void CatPlayer::ActionOne(Player* player)
{
    if(player->moveable_ == false)
	{
		return;
	}
    std::cout << "����****" << std::endl;
    player->moveable_ = false;
    player->stop_skill_time_ = CAT_ATTACK_TIME;
    player->obj_state_ = Object_State::STATE_ACTION_ONE;
    
    int session_id = player->comp_key_.session_id;

    // �ڽ� ����
    CreateAttackOBB(player, g_sessions[session_id].cat_attack_obb_);
    g_sessions[session_id].cat_attack_ = true;
    
    // �ش� ���ݿ� ���� ��
    g_sessions[session_id].CheckAttackedMice();
}


void CatPlayer::CreateAttackOBB(Player* player, DirectX::BoundingOrientedBox& box)
{
    // ĳ������ ��ġ �� ���� ����
    DirectX::XMVECTOR player_position = XMLoadFloat3(&obb_.Center);
    DirectX::XMVECTOR look = DirectX::XMVector3Normalize(XMLoadFloat3(&player->look_));

    DirectX::XMStoreFloat3(&g_sessions[player->comp_key_.session_id].cat_attack_direction_, look);

    // ���� OBB�� �߽��� ����
    // ĳ���� �� ���� ������ ���ݸ�ŭ �̵�
    DirectX::XMVECTOR center_offset = DirectX::XMVectorScale(look, attack_range);
    DirectX::XMVECTOR attack_center = DirectX::XMVectorAdd(player_position, center_offset);
    DirectX::XMStoreFloat3(&box.Center, attack_center);

    // ����̰� �� z���� ���ݸ� ���
    DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(attack_width * 2.0f, attack_height * 2.0f, attack_range);
    box.Extents = extents;

    // ���ʹϾ��� �״�� ���
    box.Orientation = obb_.Orientation;
}

void CatPlayer::InitAttackOBB(Player* player, DirectX::BoundingOrientedBox& box)
{
    // ���� OBB �ʱ�ȭ
	box.Center = DirectX::XMFLOAT3(0.0f, -9999.0f, 0.0f);
	box.Extents = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	box.Orientation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    int session_id = player->comp_key_.session_id;

    // �ش� ���ݿ� ���� ��� �ʱ�ȭ
    for(const auto& mouse : g_sessions[session_id].players_)
	{
        if (true == g_sessions[session_id].cat_attacked_player_[mouse.second->id_])
        {
            g_sessions[session_id].cat_attacked_player_[mouse.second->id_] = false;
            mouse.second->RequestUpdate();
            mouse.second->force_move_update_ = true;
        }
	}
}
