#include "stdafx.h"
#include "Sword.h"
#include "ObbComponent.h"

CSword::CSword(const CModelInfo& model, CGameObject* parent_socket) : CWeapon(model)
{
	type_ = WeaponType::Sword;

	// 검 오프셋
	XMFLOAT3 z_axis = XMFLOAT3(0, 0, 1);
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&z_axis), XMConvertToRadians(180.f));
	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, XMMatrixMultiply(XMLoadFloat4x4(&to_parent_matrix_), R));
	set_to_parent_matrix(temp);
	set_position_vector(0.f, 0.f, 110.f);

	// 검 obb 오프셋
	BoundingBox aabb(XMFLOAT3(0, 0, 0), XMFLOAT3(20, 10, 90));
	XMFLOAT4X4 obb_offset_matrix = Matrix4x4::Identity();
	obb_offset_matrix._43 += 150.f;
	CObbComponent* obb = new CObbComponent(this, aabb, parent_socket);
	obb->set_offset_matrix(obb_offset_matrix);
	AddObb(obb);

}
