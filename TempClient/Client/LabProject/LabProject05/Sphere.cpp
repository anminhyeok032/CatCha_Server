#include "stdafx.h"
#include "Sphere.h"
#include "ObbComponent.h"

CSphere::CSphere(const CModelInfo& model, CGameObject* parent_socket) : CWeapon(model)
{
	type_ = WeaponType::Sphere;

	// 창 오프셋
	XMFLOAT4X4 temp;
	XMFLOAT3 z_axis = XMFLOAT3(0, 0, 1);
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&z_axis), XMConvertToRadians(180.f));
	XMStoreFloat4x4(&temp, XMMatrixMultiply(XMLoadFloat4x4(&to_parent_matrix_), R));
	set_to_parent_matrix(temp);
	set_position_vector(0.f, 0.f, 55.f);

	// 창 obb 오프셋
	BoundingBox aabb(XMFLOAT3(0, 0, 0), XMFLOAT3(10, 10, 50));
	XMFLOAT4X4 obb_offset_matrix = Matrix4x4::Identity();
	obb_offset_matrix._43 += 100.f;
	CObbComponent* sphere_obb = new CObbComponent(this, aabb, parent_socket);
	sphere_obb->set_offset_matrix(obb_offset_matrix);
	AddObb(sphere_obb);

}
