#pragma once
#include "CharacterState.h"

class CatPlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;

	DirectX::XMFLOAT3 position_ = DirectX::XMFLOAT3(0.09281668f, 19.68505f, -3.630183f);
	DirectX::XMVECTOR center_ = DirectX::XMLoadFloat3(&position_);
	DirectX::XMFLOAT3 extents_ = DirectX::XMFLOAT3(16.57158f / 2.0f, 39.3701f / 2.0f, 62.51646f / 2.0f);
	//DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(-0.7071068f, 0.0f, 0.0f, 0.7071068f);
	DirectX::XMFLOAT4 rotation_ = DirectX::XMFLOAT4(0, 0, 0, 1);
public:
	// ������
	CatPlayer()
	{
		//extents = MathHelper::Multiply(extents, 100.0f);
		obb_ = DirectX::BoundingOrientedBox(position_, extents_, rotation_);
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// �浹 ó��
	void CheckIntersects(Player* player, float deltaTime);

	// ������ ��ȭ ������ ���� bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB ������Ʈ
	void UpdateOBB(Player* player) override;

	DirectX::BoundingOrientedBox GetOBB() override { return obb_; }
};