#pragma once
#include "global.h"
class Player;

class CharacterState {
public:
    virtual ~CharacterState() = default;

    // Ű ��ǲ ó��
    virtual void InputKey(Player* player, uint8_t key) = 0;

    // ����
    virtual void Jump(Player* player) = 0;

    // ��¡ ����
    virtual void ChargingJump(Player* player, float jump_power) = 0;

    // ��¡ ������ ���� ����� ���ʹϾ�(yaw ȸ���� �߰���)
    virtual void UpdateYaw(Player* player, float degree) = 0;

 	// �߷� ����
    virtual void ApplyGravity(Player* player, float time_step) = 0;

    // �浹 ó��
    virtual void CheckIntersects(Player* player, float deltaTime) = 0;

    // ġ����� �浹 ó��
    virtual bool CheckCheeseIntersects(Player* player, float deltaTime) = 0;

    // ���� �� ��ġ ������Ʈ
    virtual bool CalculatePhysics(Player* player, float deltaTime) = 0;

    // velocity �����ǿ� ���� �� �ִϸ��̼� ������Ʈ ����
    virtual bool CalculatePosition(Player* player, float deltaTime) = 0;

    virtual void ActionOne(Player* player) =0;
    // ��¡ ����
    virtual void ActionFourCharging(Player* player, float deltaTime) = 0;

    // OBB ������Ʈ
    virtual void UpdateOBB(Player* player) = 0;

    virtual DirectX::BoundingOrientedBox GetOBB() = 0;
};
