#include "MousePlayer.h"
#include "Player.h"
#include "MapData.h"

void MousePlayer::InputKey(Player* player, uint8_t key_)
{
	bool is_key_pressed = key_ & 0x01;
	uint8_t key_stroke = key_ >> 1;
	Action action = static_cast<Action>(key_stroke);

	//std::cout << "key input : " << (int)key << " = " << (is_key_pressed ? "true" : "false") << std::endl;

    // ��ų �������� �ƴ� ��쿡�� ��ų Ű �̺�Ʈ ó��
    if (player->moveable_ == true)
    {
        switch (action)
        {
        case Action::ACTION_ONE:
            break;
        default:
            break;
        }
    }

	// keyboard ������Ʈ
	player->SetKeyState(action, is_key_pressed);

	// ���� ������Ʈ ��û
	player->RequestSessionUpdate();
}

void MousePlayer::CheckAttack(Player* player)
{
    for (const auto& mouse_attacked : g_sessions[player->comp_key_.session_id].cat_attacked_player_)
    {
        if (mouse_attacked.second == true)
        {
            if (player->id_ == mouse_attacked.first)
            {
                return;
            }
        }
    }

    if (true == g_sessions[player->comp_key_.session_id].cat_attack_obb_.Intersects(obb_))
    {
        player->velocity_vector_.x = g_sessions[player->comp_key_.session_id].cat_attack_direction_.x * 2000.0f;
        player->velocity_vector_.y = 250.0f;
        player->velocity_vector_.z = g_sessions[player->comp_key_.session_id].cat_attack_direction_.z * 2000.0f;
        std::cout << "Cat Attack Success : mouse - " << player->id_ << std::endl;
        g_sessions[player->comp_key_.session_id].cat_attacked_player_[player->id_] = true;
    }
}

void MousePlayer::CheckIntersects(Player* player, float deltaTime)
{
    // ����� ���� üũ
    CheckAttack(player);

    // 1. ���� ��ġ�� ���� OBB �浹 �˻� (xz��, y�� �и� �˻�)
    // 2. �浹 ��, �浹�� ��ü�� ���� �̿��� �ﰢ�� ����� �ﰢ���� velocity vector�� ���� �˻� �� ���� �˻�
    // 3. ã�� �ﰢ���� ��ֺ��͸� �̿��ؼ� velocity �����ؼ� ĳ������ velocity ���͸� ����

    player->ApplyGravity(FIXED_TIME_STEP);

    // ���� ��ġ�� �ӵ��� �������� ���� ��ġ ���
    //DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&player->position_);

    // TODO : ���� �Ǻ��� �Ʒ��� �־ �߰����� velocity �������� ������. �̸� �ذ��ϱ� ���� ����̴� ���� ���� �ʿ�
    DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&obb_.Center);

    DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&player->velocity_vector_);

    float velocity_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity));
    // �ӵ��� 0�̸� �浹üũ x
    if (velocity_length < 0.0001f)
    {
        return;
    }

    DirectX::XMVECTOR slide_vector = velocity;

    // ���� OBB
    DirectX::XMVECTOR new_pos = DirectX::XMVectorAdd(currentPos, DirectX::XMVectorScale(slide_vector, deltaTime));
    DirectX::XMFLOAT3 new_pos_f3;
    DirectX::XMStoreFloat3(&new_pos_f3, new_pos);
    DirectX::BoundingOrientedBox new_obb = DirectX::BoundingOrientedBox(new_pos_f3, obb_.Extents, obb_.Orientation);

    // ��� ������Ʈ �浹üũ
    for (const auto& object : g_obbData)
    {
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(slide_vector)) < 0.0001f)
        {
            break;
        }

        // �ش� ��ü�� ���� ��ġ OBB�� �浹 �˻�
        if (false == object.second.obb.Intersects(new_obb))
        {
            continue;
        }

        //std::cout << "�浹�� : " << object.first << std::endl;

        // ���� �Ÿ��� ª�� �浹 ���������� �Ÿ�
        float min_distance = FLT_MAX;
        // ���� ����� �� normal ����
        DirectX::XMVECTOR closest_normal = DirectX::XMVectorZero();

        // �ε��� ��ü OBB ������ �迭
        DirectX::XMFLOAT3 corners[8];
        object.second.obb.GetCorners(corners);

        // **1. ��/�Ʒ� �� �˻�**
        // velocity�� y�ุ ���
        DirectX::XMVECTOR check_velocity = DirectX::XMVectorSet(0.0f, DirectX::XMVectorGetY(slide_vector), 0.0f, 0.0f);
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(check_velocity)) > 0.0001f)
        {
            DirectX::XMVECTOR normalized_velocity = DirectX::XMVector3Normalize(check_velocity);
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
                bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_velocity, p1, p2, p3, t);

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
        check_velocity = DirectX::XMVectorSet(
            DirectX::XMVectorGetX(slide_vector),
            0.0f,
            DirectX::XMVectorGetZ(slide_vector),
            0.0f);
        if (DirectX::XMVectorGetX(DirectX::XMVector3Length(check_velocity)) > 0.0001f)
        {
            DirectX::XMVECTOR normalized_velocity = DirectX::XMVector3Normalize(check_velocity);
            for (int i = 4; i < 12; ++i)
            {
                const auto& indices = g_triangle_indices[i];
                DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&corners[indices[0]]);
                DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&corners[indices[1]]);
                DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&corners[indices[2]]);

                // �ﰢ���� velocity�� ���� ���� Ȯ��
                float t;
                bool intersects = DirectX::TriangleTests::Intersects(currentPos, normalized_velocity, p1, p2, p3, t);

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

        // �����̵� ���� ���
        DirectX::XMVECTOR normalized_closest_normal = DirectX::XMVector3Normalize(closest_normal);

        // velocity�� ������ ������ �����̵� ���� ���
        // S = V - (V . N) * N
        DirectX::XMVECTOR P = DirectX::XMVectorScale(normalized_closest_normal,
            DirectX::XMVectorGetX(
                DirectX::XMVector3Dot(slide_vector, normalized_closest_normal)));
        slide_vector = DirectX::XMVectorSubtract(slide_vector, P);

        // �浹�� ����� ���̸� ���� ���̸�ŭ ���ͷ� Ƣ����� ���        
        // �浹 ���� ���� ���� ���
        float depth = CalculatePenetrationDepth(object.second, normalized_closest_normal);

        if (depth > 0.00001f) {
            // ���� ���� ���
            DirectX::XMVECTOR depth_vector = DirectX::XMVectorScale(normalized_closest_normal, depth);
            depth_vector = DirectX::XMVectorSubtract(depth_vector, DirectX::XMVectorScale(normalized_closest_normal, 0.00001f));
            DirectX::XMStoreFloat3(&player->depth_vector_, depth_vector);
            //depth_vector = DirectX::XMVectorDivide(depth_vector, DirectX::XMVectorReplicate(deltaTime));
            //slide_vector = DirectX::XMVectorAdd(slide_vector, depth_vector);
        }

        // Y��(����) �浹�� �ִϸ��̼� ������Ʈ ����
        if (DirectX::XMVectorGetY(normalized_closest_normal) > 0.9f)
        {
            player->on_ground_ = true;
            slide_vector = DirectX::XMVectorSetY(slide_vector, 0.0f);
            // ���� �� ���� ������ ���� ����� ��ȯ
            if (player->obj_state_ == Object_State::STATE_JUMP_IDLE)
            {
                player->obj_state_ = Object_State::STATE_JUMP_END;
                player->need_blending_ = true;
            }
        }

        new_pos = DirectX::XMVectorAdd(currentPos, DirectX::XMVectorScale(slide_vector, deltaTime));
        DirectX::XMStoreFloat3(&new_pos_f3, new_pos);
        new_obb.Center = new_pos_f3;

    }

    DirectX::XMStoreFloat3(&player->velocity_vector_, slide_vector);
}

float MousePlayer::CalculatePenetrationDepth(const ObjectOBB& obj, DirectX::XMVECTOR normal)
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

float MousePlayer::CalculateOBBAxisProj(DirectX::XMVECTOR axis, const DirectX::BoundingOrientedBox& obb)
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


bool MousePlayer::CalculatePhysics(Player* player, float deltaTime)
{
    bool need_update = false;

    // ���� ȸ�� üũ
    if (player->prev_player_pitch_ != player->player_pitch_)
    {
        need_update = true;
        player->prev_player_pitch_ = player->player_pitch_;
    }

    float time_remaining = (deltaTime < 1.0f) ? deltaTime : 1.0f;	// �ִ� 1�ʱ����� ���

    while (time_remaining >= FIXED_TIME_STEP)
    {
        player->delta_position_ = DirectX::XMFLOAT3();

        // �ӵ� ��� �� �̵� ���� üũ
        if (player->UpdateVelocity(FIXED_TIME_STEP))
        {
            need_update = true;

            // ���� �پ������� && �������� �ƴҶ�, �ӵ������� �Ȱų� ����
            if (player->on_ground_ == true && player->obj_state_ != Object_State::STATE_JUMP_END)
            {
                if (player->speed_ > 0.05f && player->obj_state_ == Object_State::STATE_IDLE)
                {
                    player->obj_state_ = Object_State::STATE_MOVE;
                    player->need_blending_ = true;
                }
                else if (player->speed_ <= 0.05f && player->obj_state_ == Object_State::STATE_MOVE)
                {
                    player->obj_state_ = Object_State::STATE_IDLE;
                    player->need_blending_ = true;
                }
                //player->need_blending_ = true;
            }
        }

        // ������ ����
        player->ApplyDecelerationIfStop(FIXED_TIME_STEP);
        // �� ����
        player->ApplyForces(FIXED_TIME_STEP);
        // ������ ����
        player->ApplyFriction(FIXED_TIME_STEP);
        // ��ġ ������Ʈ
        player->position_ = MathHelper::Add(player->position_, player->delta_position_);
        player->position_ = MathHelper::Add(player->position_, player->depth_vector_);
        player->depth_vector_ = DirectX::XMFLOAT3();

        //std::cout << "���� ��ġ : " << player->position_.y << std::endl;

        // ���� �ð� ���ܸ�ŭ ����
        time_remaining -= FIXED_TIME_STEP;
    }

    return need_update;
}


void MousePlayer::UpdateOBB(Player* player)
{
	// OBB ������Ʈ
    obb_.Center = player->position_;
    // pivot�� �Ʒ� �����̶� ����
    obb_.Center.y += obb_.Extents.y;
	// OBB�� ȸ�� �� ����
	obb_.Orientation = player->rotation_quat_;
}