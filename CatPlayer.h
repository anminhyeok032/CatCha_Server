#pragma once
#include "CharacterState.h"

class CatPlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;

	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3(-6.570594E-05f, 19.85514f, 1.016994f);
	DirectX::XMVECTOR center_ = DirectX::XMLoadFloat3(&position_);
	DirectX::XMFLOAT3 extents_ = DirectX::XMFLOAT3(26.47168f / 2.0f, 53.77291f / 2.0f, 39.95445f / 2.0f);
	//DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(-0.7071068f, 0, 0, 0.7071068f);
	DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);
public:
	// 생성자
	CatPlayer()
	{
		//extents = MathHelper::Multiply(extents, 100.0f);
		obb_ = DirectX::BoundingOrientedBox(position_, extents_, rotation_);
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// 충돌 처리
	void CheckIntersects(Player* player, float deltaTime);

	// 움직임 변화 감지를 위한 bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB 업데이트
	void UpdateOBB(Player* player) override;

	DirectX::BoundingOrientedBox GetOBB() override { return obb_; }
};