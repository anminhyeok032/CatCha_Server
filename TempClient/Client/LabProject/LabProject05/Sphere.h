#pragma once
#include "Weapon.h"
class CSphere :
    public CWeapon
{

public:
    CSphere(const CModelInfo& model, CGameObject* parent_socket);
};

