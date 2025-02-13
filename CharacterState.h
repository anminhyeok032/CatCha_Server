#pragma once
#include "global.h"
class Player;

class CharacterState {
public:
    virtual ~CharacterState() = default;

    // 키 인풋 처리
    virtual void InputKey(Player* player, uint8_t key) = 0;

    // 점프
    virtual void Jump(Player* player) = 0;

    // 차징 점프
    virtual void ChargingJump(Player* player, float jump_power) = 0;

    // 충돌 처리
    virtual void CheckIntersects(Player* player, float deltaTime) = 0;

    // 치즈와의 충돌 처리
    virtual bool CheckCheeseIntersects(Player* player, float deltaTime) = 0;

    // 물리 및 위치 업데이트
    virtual bool CalculatePhysics(Player* player, float deltaTime) = 0;

    virtual void ActionOne(Player* player) =0;
    // 차징 점프
    virtual void ActionFourCharging(Player* player, float deltaTime) = 0;

    // OBB 업데이트
    virtual void UpdateOBB(Player* player) = 0;

    virtual DirectX::BoundingOrientedBox GetOBB() = 0;
};
