#pragma once

#include "Weapon.h"

class CSword :
    public CWeapon
{

public:
    CSword(const CModelInfo& model, CGameObject* parent_socket);
};

