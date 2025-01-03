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


	float attack_width = extents_.x;	// 공격 범위 폭
	float attack_height = extents_.y;	// 공격 범위 높이
	float attack_range = extents_.z;	// 공격 범위 길이

public:
	// 생성자
	CatPlayer()
	{
		obb_ = DirectX::BoundingOrientedBox(position_, extents_, rotation_);
		DirectX::BoundingSphere::CreateFromBoundingBox(player_sphere_, obb_);
		player_sphere_.Radius = player_sphere_.Radius * 2.0f;
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// 충돌 처리
	void CheckIntersects(Player* player, float deltaTime);

	// 충돌시 depth 계산
	float CalculatePenetrationDepth(const ObjectOBB& obj, DirectX::XMVECTOR normal);

	float CalculateOBBAxisProj(DirectX::XMVECTOR axis, const DirectX::BoundingOrientedBox& obj_obb);

	// 움직임 변화 감지를 위한 bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB 업데이트
	void UpdateOBB(Player* player) override;

	// 공격
	void ActionOne(Player* player);
	// 공격 바운딩 박스 생성
	void CreateAttackOBB(Player* player, DirectX::BoundingOrientedBox& box);
	// 공격 바운딩 박스 초기화
	void InitAttackOBB(Player* player, DirectX::BoundingOrientedBox& box);

	DirectX::BoundingOrientedBox GetOBB() override { return obb_; }
};