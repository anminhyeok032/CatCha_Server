#pragma once
#include "CharacterState.h"

class MousePlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;
	DirectX::BoundingSphere player_sphere_;

	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3(0.00781226f, 2.38999f, 0.004617309f);
	DirectX::XMVECTOR center_ = DirectX::XMLoadFloat3(&position_);
	DirectX::XMFLOAT3 extents_ = DirectX::XMFLOAT3(3.864098f/2.0f, 5.104148f/2.0f, 10.92712f/2.0f);
	DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);

public:
	// 생성자
	MousePlayer() 
	{
		obb_ = DirectX::BoundingOrientedBox(position_, extents_, rotation_);
		DirectX::BoundingSphere::CreateFromBoundingBox(player_sphere_, obb_);
		player_sphere_.Radius = player_sphere_.Radius * 5.0f;
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	void Jump(Player* player) override;

	// 충돌 처리
	void CheckIntersects(Player* player, float deltaTime) override;

	// 치즈와의 충돌 처리
	bool CheckCheeseIntersects(Player* player, float deltaTime) override;

	// 충돌시 depth 계산
	float CalculatePenetrationDepth(const ObjectOBB& obj, DirectX::XMVECTOR normal);

	// 충돌시 축에 대한 투영 계산
	float CalculateOBBAxisProj(DirectX::XMVECTOR axis, const DirectX::BoundingOrientedBox& obb);

	// 움직임 변화 감지를 위한 bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB 업데이트
	void UpdateOBB(Player* player) override;

	void ActionOne(Player* player) override;

	DirectX::BoundingOrientedBox GetOBB() override { return obb_; }
};