#pragma once
#include "global.h"
class Player;

class CharacterState {
public:
    virtual ~CharacterState() = default;

    // 키 인풋 처리
    virtual void InputKey(Player* player, uint8_t key) = 0;

    // 물리 및 위치 업데이트
    virtual bool CalculatePhysics(Player* player, float deltaTime) = 0;
};
