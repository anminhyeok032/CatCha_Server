#pragma once
#include "Component.h"

class CGameObject;

// Owner의 전후좌우 이동을 담당
class CMovementComponent : public CComponent
{
	const XMFLOAT3 m_cxmf3GravityDirection = XMFLOAT3(0.f, -1.0f, 0.f);

	XMFLOAT3 direction_vector_ = XMFLOAT3(0.f, 0.f, 0.f);
	float acceleration_ = 8000.f;
	float friction_ = 3000.f;
	bool is_friction_ = true;

	XMFLOAT3 velocity_vector_ = XMFLOAT3(0.f, 0.f, 0.f);
	float max_speed_ = 1000.f;

	//충돌처리시 사용하기 위한 이전 위치
	XMFLOAT3 prev_position_vector_{ 0,0,0 };

	float gravity_acceleration_ = 980.f;
	float gravity_velocity_ = 0.f;

public:
	CMovementComponent() {}
	CMovementComponent(CGameObject* owner) : CComponent(owner) {}
	CMovementComponent(CGameObject* pOwner, const XMFLOAT3& xmf3Direction, const XMFLOAT3& fVelocity);
	~CMovementComponent() {}

	XMFLOAT3 prev_position_vector() const { return prev_position_vector_; }
	XMFLOAT3 direction_vector() const { return direction_vector_; }
	XMFLOAT3 velocity_vector() const { return velocity_vector_; }
	float speed() const { return Vector3::Length(velocity_vector_); }

	void set_max_speed(const float& value) { max_speed_ = value; }
	void set_direction_vector(const float& x, const float& y, const float& z) { set_direction_vector(XMFLOAT3(x, y, z)); }
	void set_direction_vector(const XMFLOAT3& xmf3Value) { direction_vector_ = xmf3Value; }
	
	virtual void Update(const float& fElapsedTime);
	void UpdateAnother(int another_num);
};

