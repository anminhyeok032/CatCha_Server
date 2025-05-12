#pragma once
#include "CharacterState.h"

class CatPlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;
	DirectX::BoundingSphere player_sphere_;

	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3(0.09281668f, 19.68505f, -3.630183f);
	DirectX::XMVECTOR center_ = DirectX::XMLoadFloat3(&position_);
	DirectX::XMFLOAT3 extents_ = DirectX::XMFLOAT3(16.57158f / 2.0f, 39.3701f / 2.0f, 62.51646f / 2.0f);
	DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);


	float attack_width = extents_.x;	// ���� ���� ��
	float attack_height = extents_.y;	// ���� ���� ����
	float attack_range = extents_.z;	// ���� ���� ����

	float total_yaw_ = 0.0f;

	DirectX::XMFLOAT4 rotation_cat_quat_ = { 0, 0, 0, 1 };						// ��¡ ������ ���� ���ʹϾ� (���� ���ʹϾ�)

public:
	// ������
	CatPlayer()
	{
		obb_ = DirectX::BoundingOrientedBox(position_, extents_, rotation_);
		DirectX::BoundingSphere::CreateFromBoundingBox(player_sphere_, obb_);
		player_sphere_.Radius = player_sphere_.Radius * 2.0f;
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	void Jump(Player* player) override;

	// �浹 ó��
	void CheckIntersects(Player* player, float deltaTime);

	// ġ����� �浹 ó��
	bool CheckCheeseIntersects(Player* player, float deltaTime) override;

	// �浹�� depth ���
	float CalculatePenetrationDepth(const ObjectOBB& obj, DirectX::XMVECTOR normal);

	float CalculateOBBAxisProj(DirectX::XMVECTOR axis, const DirectX::BoundingOrientedBox& obj_obb);

	// ������ ��ȭ ������ ���� bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// velocity �����ǿ� ���� �� �ִϸ��̼� ������Ʈ ����
	bool CalculatePosition(Player* player, float deltaTime) override;

	// OBB ������Ʈ
	void UpdateOBB(Player* player) override;

	// ����
	void ActionOne(Player* player) override;
	// ��¡ ����
	void ChargingJump(Player* player, float jump_power) override;
	void ActionFourCharging(Player* player, float deltaTime) override;
	// ��¡ ������ ���� ����� ���ʹϾ�(yaw ȸ���� �߰���)
	void UpdateYaw(Player* player, float degree) override;

	// �߷� ����
	void ApplyGravity(Player* player, float time_step) override;

	// ���� �ٿ�� �ڽ� ����
	void CreateAttackOBB(Player* player, DirectX::BoundingOrientedBox& box);
	// ���� �ٿ�� �ڽ� �ʱ�ȭ
	void InitAttackOBB(Player* player, DirectX::BoundingOrientedBox& box);

	DirectX::BoundingOrientedBox GetOBB() override { return obb_; }
};