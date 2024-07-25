#include "stdafx.h"
#include "AnotherPlayer.h"
#include "Material.h"
#include "Shader.h"
#include "MovementComponent.h"
#include "RotationComponent.h"


CAnotherPlayer::CAnotherPlayer(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, const CModelInfo& model) : CPlayer()
{

	model.heirarchy_root->AddRef();
	set_child(model.heirarchy_root);


	for (auto& p : ether_weapon_sockets_)
	{
		p = AddSocket("Bip001");
	}
	ether_weapon_sockets_[0]->set_position_vector(0, -150.f, 0);
	ether_weapon_sockets_[1]->set_position_vector(0, -100.f, 100);
	ether_weapon_sockets_[2]->set_position_vector(0, 150.f, 0);
	ether_weapon_sockets_[3]->set_position_vector(0, 100.f, 100.f);

	CGameObject* sword_socket = AddSocket("Bip001_R_Hand");
	set_weapon_socket(sword_socket);

	CreateBoneTransformMatrix(device, command_list);

	animation_controller_ = new CAnimationController(*model.animation_controller);
	
	animation_controller_->SetFrameCaches(this);

	animation_controller_->EnableTrack(0);
	animation_controller_->SetLoopType((int)PlayerAnimationState::Idle, AnimationLoopType::Repeat);
	animation_controller_->SetLoopType((int)PlayerAnimationState::SwordIdle, AnimationLoopType::Repeat);
	animation_controller_->SetLoopType((int)PlayerAnimationState::SphereIdle, AnimationLoopType::Repeat);
	animation_controller_->SetLoopType((int)PlayerAnimationState::Walk, AnimationLoopType::Repeat);
	animation_controller_->SetLoopType((int)PlayerAnimationState::Run, AnimationLoopType::Repeat);

	CMaterial* Material = new CMaterial(1);
	Material->AddTexturePropertyFromDDSFile(device, command_list, "../Resource/Model/Texture/uv.png", TextureType::RESOURCE_TEXTURE2D, 7);

	model.heirarchy_root->SetMaterial(0, Material);

	movement_component_ = new CMovementComponent((CGameObject*)this);
	rotation_component_ = new CRotationComponent((CGameObject*)this);

	movement_component_->set_max_speed(speed_);
	rotation_component_->set_use_pitch(false);


	axis_transform_matrix_ = new XMFLOAT4X4
	(1, 0, 0, 0,
	0, 0, -1, 0,
	0, 1, 0, 0,
	0, 0, 0, 1);

	CreateShaderVariables(device, command_list);

	SetShader((int)ShaderNum::Standard);

}