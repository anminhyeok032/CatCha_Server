#pragma once
#include "CharacterState.h"

class MousePlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;

	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3(0.00781226f, 2.38999f, 0.004617309f);
	DirectX::XMVECTOR center_ = DirectX::XMLoadFloat3(&position_);
	DirectX::XMFLOAT3 extents_ = DirectX::XMFLOAT3(3.864098f/2.0f, 5.104148f/2.0f, 10.92712f/2.0f);
	//DirectX::XMFLOAT3 extents_ = DirectX::XMFLOAT3(0.00781226f, 2.38999f, 0.004617309f);
	DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);


public:
	// 생성자
	MousePlayer() 
	{
		//extents = MathHelper::Multiply(extents, 100.0f);
		obb_ = DirectX::BoundingOrientedBox(position_, extents_, rotation_);
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// 고양이 공격 박스와 충돌 체크
	void CheckAttack(Player* player);

	// 충돌 처리
	void CheckIntersects(Player* player, float deltaTime) override;

	// 움직임 변화 감지를 위한 bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB 업데이트
	void UpdateOBB(Player* player) override;

	DirectX::BoundingOrientedBox GetOBB() override { return obb_; }
};