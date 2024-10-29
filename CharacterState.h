#pragma once
#include "global.h"
class Player;

class CharacterState {
public:
    virtual ~CharacterState() = default;

    // Ű ��ǲ ó��
    virtual void InputKey(Player* player, uint8_t key) = 0;

    // ���� �� ��ġ ������Ʈ
    virtual bool CalculatePhysics(Player* player, float deltaTime) = 0;
};
