#pragma once
#include "CharacterState.h"

class MousePlayer : public CharacterState
{
private:
	DirectX::BoundingOrientedBox obb_;

	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(2.2480489f, 8.8407925f, 3.3379889f);
	DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

public:
	// 생성자
	MousePlayer() 
	{
		obb_ = DirectX::BoundingOrientedBox(position, extents, rotation);
	}

	// KeyInput
	void InputKey(Player* player, uint8_t key) override;

	// 충돌 처리
	void CheckIntersects(Player* player, float deltaTime) override;

	// 움직임 변화 감지를 위한 bool return
	bool CalculatePhysics(Player* player, float deltaTime) override;

	// OBB 업데이트
	void UpdateOBB(Player* player) override;
};