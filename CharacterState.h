#pragma once
#include "global.h"
class Player;

class CharacterState {
public:
    virtual ~CharacterState() = default;

    // 키 인풋 처리
    virtual void InputKey(Player* player, uint8_t key) = 0;

    // 충돌 처리
    virtual void CheckIntersects(Player* player, float deltaTime) = 0;

    // 물리 및 위치 업데이트
    virtual bool CalculatePhysics(Player* player, float deltaTime) = 0;

    // OBB 업데이트
    virtual void UpdateOBB(Player* player) = 0;

    virtual DirectX::BoundingOrientedBox GetOBB() = 0;
};
