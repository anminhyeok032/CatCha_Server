#include "stdafx.h"
#include "Weapon.h"

CWeapon::CWeapon(const CModelInfo& model) : CRootObject(model)
{
	is_visible_ = false;
}

CWeapon::CWeapon(const CWeapon& other) : CRootObject(other), type_(other.type_), name_(other.name_)
{

}
