#pragma once
#include "CharacterState.h"

class CatPlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;
public:
	// ������
	CatPlayer()
	{
		DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(10.5483342f, 15.622265f, 30.9288589f);
		DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		obb_ = DirectX::BoundingOrientedBox(position, extents, rotation);
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// �浹 ó��
	void CheckIntersects(Player* player, float deltaTime);

	// ������ ��ȭ ������ ���� bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB ������Ʈ
	void UpdateOBB(Player* player) override;
};