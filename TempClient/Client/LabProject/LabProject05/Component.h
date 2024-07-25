#pragma once

class CGameObject;

class CComponent
{
protected:
	CGameObject* owner_ = nullptr;

public:
	CComponent() {}
	CComponent(CGameObject* pOwner) { owner_ = pOwner; }
	~CComponent() {}
	
	virtual void Update(const float& fElapsedTime) {}
};

