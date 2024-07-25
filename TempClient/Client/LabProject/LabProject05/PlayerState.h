#pragma once
#include "State.h"

class CPlayer;


class PIdle : public State<CPlayer>
{
    const float release_weapon_time_ = 3.f;
    float idle_time_ = 0.f;
public:
    static PIdle* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer*, float elapsed);

    virtual void Exit(CPlayer* player);

};

class PMove : public State<CPlayer>
{
public:
    static PMove* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);

};

class PEvade : public State<CPlayer>
{
private:
    //5프레임 부터 이동 시작
    const float move_start_time_ = 1.f / 60.f * 5;
    float evade_time_ = 0.f;

public:
    static PEvade* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);

};

class PAttack : public State<CPlayer>
{
public:
    virtual void Enter(CPlayer* player);
};

class PAttack1 : public PAttack
{
public:
    static PAttack1* Instance(const WeaponType& type);
};

class PAttack2 : public PAttack
{
public:
    static PAttack2* Instance(const WeaponType& type);
};

class PAttack3 : public PAttack
{
public:
    static PAttack3* Instance(const WeaponType& type);
};

class PAttack4 : public PAttack
{
public:
    static PAttack4* Instance(const WeaponType& type);
};


//TODO: 여유가 있다면 사전동작이 캐릭터를 살짝 뒤로 가게하면 좋을듯
class PSwordAttack1 : public PAttack1
{
private:
    const float max_charge_time_ = 2.5f;
    float charge_time_ = 0.f;
    bool is_charging_ = false;

    const float slash_move_start_time_ = 1.f / 60.f * 12.f;
    const float slash_move_end_time_ = 1.f / 60.f * 28.f;
    const float slash_move_speed_ = 1000.f;
    float slash_time_ = 0.f;

public:
    static PSwordAttack1* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);

};

class PSwordAttack2 : public PAttack2
{
private:
    const float slash_move_speed_ = 700.f;
    const float max_hold_time_ = 4.0f;
    float hold_time_ = 0.f;
    bool is_holding_ = false;

public:
    static PSwordAttack2* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);

};

class PSwordAttack3 : public PAttack3
{
private:
    const std::array<float, 2> move_start_time_ = { 1.f / 30.f * 10.f, 1.f / 30.f * 20.f };
    const std::array<float, 2> move_end_time_ = { 1.f / 30.f * 20.f, 1.f / 30.f * 30.f };
    const std::array<float, 2> move_speed_ = { 1500.f, 1000.f };

    float attack_time_ = 0.f;

public:
    static PSwordAttack3* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);

};

class PSwordAttack4 : public PAttack4
{
private:
    const float weapon_invisible_time_ = 1.f / 30.f * 35.f;
    const std::array<float, 2> move_start_time_ = { 0.f, 1.f / 30.f * 30.f };
    const std::array<float, 2> move_end_time_ = { 1.f / 30.f * 5.f, 1.f / 30.f * 40.f };
    const std::array<float, 2> move_speed_ = { 240.f, 120.f };
   
    float attack_time_ = 0.f;

public:
    static PSwordAttack4* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);

};

class PSphereAttack1 : public PAttack1
{
private:
    const std::array<float, 2> move_start_time_ = { 1.f / 30.f * 3.f, 1.f / 30.f * 25.f };
    const std::array<float, 2> move_end_time_ = { 1.f / 30.f * 10.f, 1.f / 30.f * 35.f };
    const std::array<float, 2> move_speed_ = { 1000.f, 1500.f };

    float attack_time_ = 0.f;

    bool is_click_ = false;

public:
    static PSphereAttack1* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);
};

class PSphereAttack2 : public PAttack2
{
private:
    const std::array<float, 2> move_start_time_ = { 1.f / 30.f * 10.f, 1.f / 30.f * 38.f };
    const std::array<float, 2> move_end_time_ = { 1.f / 30.f * 38.f, 1.f / 30.f * 55.f };
    const std::array<float, 2> move_speed_ = { 1000.f, 500.f };

    float attack_time_ = 0.f;

public:
    static PSphereAttack2* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);
};

class PSphereAttack3 : public PAttack3
{
private:
    const float move_start_time_ = 1.f / 30.f * 10.f;
    const float move_end_time_ = 1.f / 30.f * 29.f;
    const float move_speed_ = 800.f;

    float attack_time_ = 0.f;

    const float max_charge_time_ = 2.f;
    bool is_charging_ = true;

    float charge_time_ = 0.f;

public:
    static PSphereAttack3* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);
};

class PSphereAttack4 : public PAttack4
{
private:
    const std::array<float, 3> move_start_time_ =   { 1.f / 30.f * 0.f, 1.f / 30.f * 20.f, 1.f / 30.f * 40 };
    const std::array<float, 3> move_end_time_ =     { 1.f / 30.f * 5.f, 1.f / 30.f * 30.f, 1.f / 30.f * 50 };
    const std::array<float, 3> move_speed_ = { 500.f, 3000.f, 200.f };

    float attack_time_ = 0.f;

public:
    static PSphereAttack4* Instance();

public:
    virtual void Enter(CPlayer* player);

    virtual void Execute(CPlayer* player, float elapsed_time);

    virtual void Exit(CPlayer* player);
};