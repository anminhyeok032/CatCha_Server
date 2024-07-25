#include "stdafx.h"
#include "Scene.h"
#include "Shader.h"
#include "Camera.h"
#include "Player.h"
#include "Object.h"
#include "DescriptorManager.h"
#include "AudioManager.h"
#include "AnimationCallbackFunc.h"
#include "Mawang.h"
#include "SkyBox.h"
#include "Mesh.h"
#include "Sword.h"
#include "Sphere.h"
#include "Building.h"
#include "ObbComponent.h"
#include "AnotherPlayer.h"


//extern std::unordered_map<int, bool> g_objects;

CScene::CScene()
{

}

CScene::~CScene()
{
}


void CScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights->m_pLights[1].m_bEnable = true;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 60.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(-150.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 8.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

}


#define _WITH_OBJECT_LIGHT_MATERIAL_DESCRIPTOR_TABLE

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[5];
	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	pd3dDescriptorRanges[0].BaseShaderRegister = 4; //Lights
	pd3dDescriptorRanges[0].NumDescriptors = 1;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	pd3dDescriptorRanges[1].BaseShaderRegister = 17; //Texture(detail)
	pd3dDescriptorRanges[1].NumDescriptors = 1;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	pd3dDescriptorRanges[2].BaseShaderRegister = 18; //Texture(base)
	pd3dDescriptorRanges[2].NumDescriptors = 1;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	pd3dDescriptorRanges[3].BaseShaderRegister = 0; //Texture(DiffuseColor)
	pd3dDescriptorRanges[3].NumDescriptors = 1;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	pd3dDescriptorRanges[4].BaseShaderRegister = 19; //Texture(SkyBox)
	pd3dDescriptorRanges[4].NumDescriptors = 1;


	D3D12_ROOT_PARAMETER pd3dRootParameters[9];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 16; //Game Objects
	pd3dRootParameters[1].Constants.ShaderRegister = 1;
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Lights
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1; //Terrain Textures
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[4].Descriptor.ShaderRegister = 5; //BoneOffsets
	pd3dRootParameters[4].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[5].Descriptor.ShaderRegister = 6; //BoneTransforms
	pd3dRootParameters[5].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1; //Terrain Textures
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1; //DiffuseColor Texuture
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1; //SkyBox Texuture
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDescs[2];
	d3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].MipLODBias = 0;
	d3dSamplerDescs[0].MaxAnisotropy = 1;
	d3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[0].MinLOD = 0;
	d3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[0].ShaderRegister = 0;
	d3dSamplerDescs[0].RegisterSpace = 0;
	d3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[1].MipLODBias = 0;
	d3dSamplerDescs[1].MaxAnisotropy = 1;
	d3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[1].MinLOD = 0;
	d3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[1].ShaderRegister = 1;
	d3dSamplerDescs[1].RegisterSpace = 0;
	d3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 2;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}


void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 조명정보
	UINT ncbLightsBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	d3d12_lights_ = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbLightsBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	d3d12_lights_->Map(0, NULL, (void**)&mapped_lights_);

	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dcbvDesc;
	d3dcbvDesc.BufferLocation = d3d12_lights_->GetGPUVirtualAddress();
	d3dcbvDesc.SizeInBytes = ncbLightsBytes;
	pd3dDevice->CreateConstantBufferView(&d3dcbvDesc, descriptor_manager_->GetCbvNextCPUHandle());
	d3d12_lights_gpu_descriptor_start_handle_ = descriptor_manager_->GetCbvNextGPUHandle();

	descriptor_manager_->AddCbvIndex();
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(mapped_lights_, m_pLights, sizeof(LIGHTS));
}

void CScene::ReleaseShaderVariables()
{
	if (d3d12_lights_)
	{
		d3d12_lights_->Unmap(0, NULL);
		d3d12_lights_->Release();
	}
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice)
{
	player_->CreateShaderResourceViews(pd3dDevice, descriptor_manager_);

	terrain_->CreateShaderResourceViews(pd3dDevice, descriptor_manager_);

	skybox_->CreateShaderResourceViews(pd3dDevice, descriptor_manager_);


	for (auto& Object : objects_)
	{
		Object->CreateShaderResourceViews(pd3dDevice, descriptor_manager_);
	}
	for (auto& another : anothers_)
	{
		another->CreateShaderResourceViews(pd3dDevice, descriptor_manager_);
	}

}

void CScene::ReleaseUploadBuffers()
{
	for (auto& pObject : objects_) pObject->ReleaseUploadBuffers();
	for (auto& pAnother : anothers_) pAnother->ReleaseUploadBuffers();
}

void CScene::AnimateObjects(float elapsed_time)
{
	
	if (m_pLights)
	{
		m_pLights->m_pLights[1].m_xmf3Position = player_->position_vector();
		m_pLights->m_pLights[1].m_xmf3Direction = player_->look_vector();
	}
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CPlayer* pPlayer, CAudioManager* audio_manager)
{
	audio_manager_ = audio_manager;

	player_ = pPlayer;

	d3d12_root_signature_ = CreateGraphicsRootSignature(pd3dDevice);

	int shader_num = 5;
	shaders_.reserve(shader_num);

	shaders_.emplace_back(new CStandardShader);
	shaders_.emplace_back(new CTerrainShader);
	shaders_.emplace_back(new CStaticMeshShader);
	shaders_.emplace_back(new CSkyBoxShader);
	shaders_.emplace_back(new CDiffusedShader);


	for(auto& shader : shaders_)
		shader->CreateShader(pd3dDevice, d3d12_root_signature_);


	descriptor_manager_ = new CDescriptorManager;
	descriptor_manager_->SetDescriptors(1 + 20 + 1); // 조명(cbv), 터레인(2) 모델텍스처(3), 스카이박스(1)

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = descriptor_manager_->GetDescriptors();		
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	ID3D12DescriptorHeap* pd3dDescritorHeap;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pd3dDescritorHeap);

	descriptor_manager_->SetDesriptorHeap(pd3dDescritorHeap);
	descriptor_manager_->Initialization(1);

	//TODO: Light 관련 차후 수정
	BuildLightsAndMaterials();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	XMFLOAT3 xmf3Scale(10.0f, 1.f, 10.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	terrain_ = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, d3d12_root_signature_, _T("../Resource/Terrain/main_8.raw"), 1025, 1025, 1025, 1025, xmf3Scale, xmf4Color);
	
	skybox_ = new CSkyBox(pd3dDevice, pd3dCommandList);
	shaders_[3]->AddObject(skybox_);

	shaders_[1]->AddObject(terrain_);

	player_->set_position_vector(500, terrain_->GetHeight(500, 500), 500);
	player_->SetAnimationCallbackKey((int)PlayerAnimationState::Run, 0.1, new CSoundCallbackFunc(audio_manager_, "Footstep01"));
	player_->SetShader(4);

	XMFLOAT3 extents(15, 25, 87.5);
	CGameObject* collision_socket = player_->AddSocket("Bip001");
	
	BoundingBox aabb(XMFLOAT3(0,0,0), extents);
	player_->AddObb(aabb, collision_socket);
	shaders_[4]->AddObject(collision_socket);
	shaders_[0]->AddObject(player_);


	dynamic_object_list_.push_back(player_);


	// 04.30 수정: 플레이어 객체와 터레인 객체는 따로관리(충돌체크 관리를 위해)
	//=========================================
	// Another Player
	//=========================================
	for (int i = 0; i < MAX_USER; ++i)
	{
		CModelInfo another = CGameObject::LoadModelInfoFromFile(pd3dDevice, pd3dCommandList, "../Resource/Model/Player_Model.bin");
		CAnotherPlayer* object = new CAnotherPlayer(pd3dDevice, pd3dCommandList, another);
		object->set_position_vector(-9999, -999, -9999);
		object->set_look_vector(0.f, 0.f, 1.f);
		
		collision_socket = object->AddSocket("Bip001");
		object->AddObb(aabb, collision_socket);
		PlayerAnimationState a = PlayerAnimationState::Roll;
		object->set_animation_state(a);
		anothers_.push_back(object);
		

		/*CGameObject* sword_socket = player_->AddSocket("Bip001_R_Hand");
		object->set_weapon_socket(sword_socket);*/

		// player_를 따로 만드는건 내부에 player_객체가 두개 인거라 비효율
		/*object->player_->set_current_weapon(WeaponType::None);
		object->player_->state_machine()->ChangeState(PMove::Instance());*/

		shaders_[0]->AddObject(object);
		dynamic_object_list_.push_back(object);
	}

	//=========================================

	CGameObject* sword_socket = player_->AddSocket("Bip001_R_Hand");

	CModelInfo model = CGameObject::LoadModelInfoFromFile(pd3dDevice, pd3dCommandList, "../Resource/Model/Weapons/Sword_TXT.bin");
	CWeapon* weapon = new CSword(model, sword_socket);

	weapon->set_name("default_sword");

	weapon->SetShader((int)ShaderNum::StaticMesh);
	player_->AddWeapon(weapon);
	objects_.push_back(weapon);
	shaders_[2]->AddObject(sword_socket);
	dynamic_object_list_.push_back(weapon);

	player_->SetEtherWeaponSocketByShader(shaders_[2]);

	model = CGameObject::LoadModelInfoFromFile(pd3dDevice, pd3dCommandList, "../Resource/Model/Weapons/Sphere_TXT.bin");
	weapon = new CSphere(model, sword_socket);
	weapon->set_name("default_sphere");
	weapon->SetShader((int)ShaderNum::StaticMesh);

	player_->AddWeapon(weapon);
	objects_.push_back(weapon);
	dynamic_object_list_.push_back(weapon);

	player_->set_weapon_socket(sword_socket);

	

	model = CGameObject::LoadModelInfoFromFile(pd3dDevice, pd3dCommandList, CMawang::mawang_model_file_name_);
	
	CRootObject* object = new CMawang(pd3dDevice, pd3dCommandList, model);
	object->set_position_vector(750, terrain_->GetHeight(750, 550), 550);

	collision_socket = object->AddSocket("Bip001");
	object->AddObb(aabb, collision_socket);
	objects_.push_back(object);
	shaders_[0]->AddObject(object);

	dynamic_object_list_.push_back(object);

	object = new CMawang(pd3dDevice, pd3dCommandList, model);
	object->set_position_vector(50, terrain_->GetHeight(50, 550), 550);
	object->AddObb(aabb, collision_socket);
	objects_.push_back(object);
	shaders_[0]->AddObject(object);

	dynamic_object_list_.push_back(object);

	model = CGameObject::LoadModelInfoFromFile(pd3dDevice, pd3dCommandList, "../Resource/Model/Building/Test_TXT.bin");

	object = new CBuilding(model);
	collision_socket = object->AddSocket(object);
	aabb.Extents = XMFLOAT3(500, 500, 200);
	collision_socket->set_position_vector(0, 0, 200);
	object->AddObb(aabb, collision_socket);
	object->set_position_vector(2000, terrain_->GetHeight(2000, 600), 600);
	object->Rotate(0, 45, 0);
	objects_.push_back(object);
	object->SetShader((int)ShaderNum::StaticMesh);
	shaders_[2]->AddObject(object);

	static_object_list_.push_back(object);

	CreateShaderResourceViews(pd3dDevice); // 모든 오브젝트의 Srv 생성

	player_->EquipWeapon("default_sphere");
	
	// obb 업데이트를 위한 사전 animate
	for (auto& p : dynamic_object_list_)
	{
		p->Animate(0);
	}
	for (auto& p : static_object_list_)
	{
		p->Animate(0);
	}

	CreateCollisionCubeMesh(pd3dDevice, pd3dCommandList);

}

void CScene::ReleaseObjects()
{
	if (d3d12_root_signature_) d3d12_root_signature_->Release();

	delete player_;
	player_ = NULL;

	delete terrain_;
	terrain_ = NULL;

	delete skybox_;
	skybox_ = NULL;


	for (auto& pObject : objects_)
	{
		delete pObject;
		pObject = NULL;
	}

	for (auto& pAnother : anothers_)
	{
		delete pAnother;
		pAnother = NULL;
	}


	for (auto& shader : shaders_)
	{
		delete shader;
		shader = NULL;
	}

	if (descriptor_manager_) delete descriptor_manager_;

	ReleaseShaderVariables();

	if (m_pLights) delete m_pLights;
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, float elapsed_time)
{
	pd3dCommandList->SetGraphicsRootSignature(d3d12_root_signature_);
	pd3dCommandList->SetDescriptorHeaps(1, &descriptor_manager_->GetDescriptorHeap());

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

#ifdef _WITH_OBJECT_LIGHT_MATERIAL_DESCRIPTOR_TABLE
	//pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dMaterialsCbvGPUDescriptorHandle);
#else
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = d3d12_lights_->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(4, d3dcbLightsGpuVirtualAddress); //Lights

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbMaterialsGpuVirtualAddress);
#endif

	//05.08 수정: 이제 셰이더는 그 셰이더를 사용하는 오브젝트들을 렌더함
	for (auto& shader : shaders_)
	{
		shader->Render(pd3dCommandList, pCamera, elapsed_time);

		if (debug_collision_ && shader->GetShaderNum() == (int)ShaderNum::Diffused)
		{
			for (auto& object : dynamic_object_list_)
			{
				object->RenderObbList(pd3dCommandList);
			}
			for (auto& object : static_object_list_)
			{
				object->RenderObbList(pd3dCommandList);
			}
		}
	}


}

void CScene::UpdateCollisionList()
{
}

void CScene::UpdatePlayerlist(float elapsed_time)
{
	if (player_->GetMyId() != g_myid)	player_->set_my_id(g_myid);
	for (int i = 0; i < MAX_USER; i++)
	{
		if (g_myid == i)	continue;
		if (g_objects.count(i) == 1)
		{
			anothers_[i]->set_my_id(i);
			anothers_[i]->set_position_vector(g_objects[i].Location);	
		}
		if (anothers_[i]->position_vector().y != -999)
		{
			anothers_[i]->UpdateAnothers(elapsed_time, i);
			anothers_[i]->OrientRotationAnothers(i);
			//anothers_[i]->Rotate(0, g_objects[i].yaw, 0);
			//anothers_[i]->state_machine()->ChangeState(a);
		}
	}
}

void CScene::CollisionCheck()
{
	// 지면에 닿아있는지 체크
	XMFLOAT3 position = player_->position_vector();
	float terrain_height = terrain_->GetHeight(position.x, position.z);
	if (position.y > terrain_height)
		player_->set_is_fall(true);
	if (IsEqual(position.y, terrain_height))
		player_->set_is_fall(false);
	if (position.y < terrain_height)
	{
		player_->set_position_vector(position.x, terrain_height, position.z);
		player_->set_is_fall(false);
	}
		

	//TODO: 객체의 OBB를 이용해서 서로 겹치면 밀어내는 기본 충돌처리 구현
	for (auto p = dynamic_object_list_.begin(); p != dynamic_object_list_.end(); ++p)
	{
		for (auto other = p; other != dynamic_object_list_.end(); ++other)
		{
			// 충돌한 소켓을 찾기위한 객체
			CObbComponent a, b;
			if (other != p && CRootObject::CollisionCheck(*p, *other, a, b))
			{
				(*p)->HandleCollision(*other, a, b);
				(*other)->HandleCollision(*p, b, a);
			}			
		}
	}

	for (auto& p : dynamic_object_list_)
	{
		for (auto& other : static_object_list_)
		{
			// 충돌한 소켓을 찾기위한 객체
			CObbComponent a, b;
			if (CRootObject::CollisionCheck(p, other, a, b))
			{
				p->HandleCollision(other, a, b);
				other->HandleCollision(p, b, a);
			}
		}
	}

}

void CScene::CreateCollisionCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	for (auto& object : static_object_list_)
	{
		object->CreateCollisionCubeMesh(device, command_list);
	}

	for (auto& object : dynamic_object_list_)
	{
		object->CreateCollisionCubeMesh(device, command_list);
	}

}

