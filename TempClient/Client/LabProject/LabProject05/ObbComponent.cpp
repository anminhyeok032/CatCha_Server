#include "stdafx.h"
#include "ObbComponent.h"
#include "Object.h"
#include "Mesh.h"

CObbComponent::CObbComponent(const CObbComponent& other) : 
	obb_ (other.obb_),
	animated_obb_(other.animated_obb_),
	world_matrix_(other.world_matrix_),
	parent_socket_(other.parent_socket_)
{
	// 디버그 메쉬는 복사하지 않음
	debug_cube_ = NULL;
}

CObbComponent::CObbComponent(CGameObject* owner, const BoundingBox& aabb, CGameObject* parent_socket) : CComponent(owner)
{
	BoundingOrientedBox::CreateFromBoundingBox(obb_, aabb);
	parent_socket_ = parent_socket;
}

CObbComponent::~CObbComponent()
{
	if (debug_cube_)
		delete debug_cube_;
}

void CObbComponent::CreateDebugCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	//이미 디버깅 큐브가 있으면 생성 X
	if (debug_cube_) return;
	debug_cube_ = new CCubeMesh(device, command_list, obb_.Center - obb_.Extents, obb_.Extents);
}

void CObbComponent::Update()
{
	if (parent_socket_)
	{
		world_matrix_ = offset_matrix_ * parent_socket_->GetWorldMatrix();
		obb_.Transform(animated_obb_, XMLoadFloat4x4(&world_matrix_));
	}
}

void CObbComponent::Render(ID3D12GraphicsCommandList* command_list)
{
	if (!debug_cube_) return;
	if (!is_active_) return;

	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&world_matrix_)));
	command_list->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);

	debug_cube_->Render(command_list);
}

bool CObbComponent::Intersects(const BoundingOrientedBox& other)
{
	return animated_obb_.Intersects(other);
}
