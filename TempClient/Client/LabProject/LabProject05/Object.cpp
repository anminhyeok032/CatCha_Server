#include "stdafx.h"
#include "Object.h"
#include "Scene.h"
#include "Shader.h"
#include "Animation.h"
#include "Camera.h"
#include "Mesh.h"
#include "DescriptorManager.h"
#include "Material.h"
#include "ObbComponent.h"

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||| <CGameObject> |||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CGameObject::CGameObject()
{
	to_parent_matrix_ = Matrix4x4::Identity();
	to_prev_matrix_ = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::CGameObject(ObjectType object_type)
{
	to_parent_matrix_ = Matrix4x4::Identity();
	to_prev_matrix_ = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
	// add한 오브젝트 로드를 위한
	switch (object_type)
	{
	case OT_PLAYER:
		// TODO: 여기서 캐릭터의 모델을 로드하도록
		//CModelInfo model = CGameObject::LoadModelInfoFromFile(pd3dDevice, pd3dCommandList, CMawang::mawang_model_file_name_)
		//this = new CGameObject(model);
		break;
	}
}

CGameObject::~CGameObject()
{
	for (auto& p : meshes_) p->Release();
	for (auto& p : m_Materials) p->Release();
	if (axis_transform_matrix_) delete axis_transform_matrix_;

	if (child_) child_->Release();
	if (sibling_) sibling_->Release();
}

void CGameObject::set_look_vector(const float& x, const float& y, const float& z)
{
	to_parent_matrix_._31 = x, to_parent_matrix_._32 = y, to_parent_matrix_._33 = z;
}

void CGameObject::set_right_vector(const float& x, const float& y, const float& z)
{
	to_parent_matrix_._11 = x, to_parent_matrix_._12 = y, to_parent_matrix_._13 = z;
}

void CGameObject::set_up_vector(const float& x, const float& y, const float& z)
{
	to_parent_matrix_._21 = x, to_parent_matrix_._22 = y, to_parent_matrix_._23 = z;
}

void CGameObject::SetMesh(CMesh* mesh)
{
	meshes_.push_back(mesh);
}

void CGameObject::AddMaterial(CMaterial* pMaterial)
{
	m_Materials.emplace_back(pMaterial);
	m_Materials.back()->AddRef();

	meshes_[m_Materials.size() - 1]->set_material(pMaterial);
}


void CGameObject::SetMaterial(const int& index, CMaterial* pMaterial)
{
	if (m_Materials.size() == index)
	{
		m_Materials.emplace_back(pMaterial);
	}
	else 
		m_Materials[index] = pMaterial;
	m_Materials[index]->AddRef();

	meshes_[index]->set_material(pMaterial);
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	if (axis_transform_matrix_)
	{
		XMFLOAT4X4 mat = Matrix4x4::Multiply(*axis_transform_matrix_, to_parent_matrix_);
		m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(mat, *pxmf4x4Parent) : mat;
	}
	else
		m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(to_parent_matrix_, *pxmf4x4Parent) : to_parent_matrix_;

	if (sibling_) sibling_->UpdateTransform(pxmf4x4Parent);
	if (child_) child_->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager)
{
	for (auto& Material : m_Materials)
		Material->CreateShaderResourceViews(pd3dDevice, pDescriptorManager);

	if (child_) child_->CreateShaderResourceViews(pd3dDevice, pDescriptorManager);
	if (sibling_) sibling_->CreateShaderResourceViews(pd3dDevice, pDescriptorManager);
}

void CGameObject::Animate(float fTimeElapsed)
{

	if (sibling_) sibling_->Animate(fTimeElapsed);
	if (child_) child_->Animate(fTimeElapsed);
}

void CGameObject::ResetAnimatedSRT()
{
	m_xmf3BlendedScale = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3BlendedRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3BlendedTranslation = XMFLOAT3(0.0f, 0.0f, 0.0f);

	if (sibling_) sibling_->ResetAnimatedSRT();
	if (child_) child_->ResetAnimatedSRT();

}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int shader_num)
{
	if (shader_num != shader_num_) return;
	if (!is_visible_) return;

	// 05.17: 이제 텍스처는 적용되는 메쉬에서 렌더전에 set함
	//for(auto& p : m_Materials)
	//	p->UpdateShaderVariables(pd3dCommandList);


	for (auto& p : meshes_)
	{
		UpdateShaderVariables(pd3dCommandList);
		p->Render(pd3dCommandList);
	}

	if (child_) child_->Render(pd3dCommandList, pCamera, shader_num);
	if (sibling_) sibling_->Render(pd3dCommandList, pCamera, shader_num);
}

void CGameObject::ReleaseUploadBuffers()
{
	for (auto& p : meshes_)
		p->ReleaseUploadBuffers();

	for (auto& p : m_Materials)
		p->ReleaseUploadBuffers();
}

void CGameObject::SetBoneFrameCaches(CGameObject** bone_frame_caches, int bone_count)
{
	bone_count_ = bone_count;
	bone_frame_caches_ = bone_frame_caches;
}

void CGameObject::set_position_vector(const float& x, const float& y, const float& z)
{
	to_parent_matrix_._41 = x;
	to_parent_matrix_._42 = y;
	to_parent_matrix_._43 = z;
}

void CGameObject::set_prev_position_vector(const float& x, const float& y, const float& z)
{
	to_prev_matrix_._41 = x;
	to_prev_matrix_._42 = y;
	to_prev_matrix_._43 = z;
}


XMFLOAT3 CGameObject::position_vector() const
{
	return(XMFLOAT3(to_parent_matrix_._41, to_parent_matrix_._42, to_parent_matrix_._43));
}

XMFLOAT3 CGameObject::look_vector() const
{
	return(Vector3::Normalize(XMFLOAT3(to_parent_matrix_._31, to_parent_matrix_._32, to_parent_matrix_._33)));
}

XMFLOAT3 CGameObject::up_vector() const
{
	return(Vector3::Normalize(XMFLOAT3(to_parent_matrix_._21, to_parent_matrix_._22, to_parent_matrix_._23)));
}

XMFLOAT3 CGameObject::right_vector() const
{
	return(Vector3::Normalize(XMFLOAT3(to_parent_matrix_._11, to_parent_matrix_._12, to_parent_matrix_._13)));
}

void CGameObject::SetShader(const int& nShader)
{ 
	shader_num_ = nShader;

	if (child_) child_->SetShader(nShader);
	if (sibling_) sibling_->SetShader(nShader);
}

void CGameObject::UpdateMatrixByBlendedSRT()
{
	XMMATRIX S = XMMatrixScaling(m_xmf3BlendedScale.x, m_xmf3BlendedScale.y, m_xmf3BlendedScale.z);
	XMMATRIX R = XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(m_xmf3BlendedRotation.x), XMMatrixRotationY(m_xmf3BlendedRotation.y)), XMMatrixRotationZ(m_xmf3BlendedRotation.z));
	XMMATRIX T = XMMatrixTranslation(m_xmf3BlendedTranslation.x, m_xmf3BlendedTranslation.y, m_xmf3BlendedTranslation.z);
	XMStoreFloat4x4(&to_parent_matrix_, XMMatrixMultiply(XMMatrixMultiply(S, R), T));
}

void CGameObject::set_child(CGameObject* ptr)
{
	if (ptr)
	{
		ptr->set_parent(this);
	}
	if (child_)
	{
		if (ptr) ptr->sibling_ = child_->sibling_;
		child_->sibling_ = ptr;
	}
	else
	{
		child_ = ptr;
	}
}

void CGameObject::set_sibling(CGameObject* ptr)
{
	if (ptr)
	{
		if(parent_)
			ptr->parent_ = parent_;
		if (sibling_)
		{
			sibling_->set_sibling(ptr);
		}
		else
		{
			sibling_ = ptr;
		}
	}
}

void CGameObject::set_parent(CGameObject* ptr)
{
	parent_ = ptr;
	if (sibling_) sibling_->set_parent(ptr);
}

CGameObject* CGameObject::FindFrame(const std::string& strFrameName)
{
	if (m_strFrameName == strFrameName) return this;

	CGameObject* pFrameObject = NULL;
	if (sibling_) if (pFrameObject = sibling_->FindFrame(strFrameName)) return(pFrameObject);
	if (child_) if (pFrameObject = child_->FindFrame(strFrameName)) return(pFrameObject);

	return NULL;
}

void CGameObject::PrepareSkinning(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pRootObject)
{
	for (auto& p : meshes_)
	{
		CSkinMesh* pSkinMesh = dynamic_cast<CSkinMesh*>(p);
		if (pSkinMesh)
			pSkinMesh->SetBoneFrameCaches(pd3dDevice, pd3dCommandList, pRootObject);
	}

	if (child_) child_->PrepareSkinning(pd3dDevice, pd3dCommandList, pRootObject);
	if (sibling_) sibling_->PrepareSkinning(pd3dDevice, pd3dCommandList, pRootObject);
}

CSocket* CGameObject::AddSocket(CGameObject* parent_frame)
{
	CSocket* socket = new CSocket();
	CGameObject* socket_parent = parent_frame;
	socket_parent->set_child(socket);

	return socket;
}

CSocket* CGameObject::AddSocket(const std::string& frame_name)
{
	CSocket* socket = new CSocket();
	CGameObject* socket_parent = FindFrame(frame_name);
	socket_parent->set_child(socket);

	return socket;
}

void CGameObject::LoadMaterialFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile, CGameObject* root_object)
{
	int nMaterials = FBXLoad::ReadFromFile<int>(InFile);
	m_Materials.reserve(nMaterials);

	std::string strToken;

	for (int i = 0; i < nMaterials; ++i)
	{

		FBXLoad::ReadStringFromFile(InFile, strToken);

		FBXLoad::ReadFromFile<int>(InFile);

		FBXLoad::ReadStringFromFile(InFile, strToken);

		CMaterial* pMaterial = new CMaterial; //TODO: Material의 이름으로 중복이 있는지 체크해야함

		pMaterial->SetName(strToken);

		FBXLoad::ReadStringFromFile(InFile, strToken); // <TextureProperties>: 

		pMaterial->LoadTexturePropertiesFromFile(pd3dDevice, pd3dCommandList, InFile);

		root_object->AddMaterial(pMaterial);

		FBXLoad::ReadStringFromFile(InFile, strToken); // </TextureProperties>

		FBXLoad::ReadStringFromFile(InFile, strToken); // <ShadingModel>:

		FBXLoad::ReadStringFromFile(InFile, strToken); // name
		std::string strName = strToken;

		FBXLoad::ReadStringFromFile(InFile, strToken); // <Phong>: or <Lambert>:, <Unknown>
		if (strToken == "<Phong>:")
		{
			CShadingModel* pShadingModel = new CPhongModel;
			pShadingModel->LoadShadingModelFromFile(InFile);
		}
		else if (strToken == "<Lambert>:")
		{
			CShadingModel* pShadingModel = new CLambertModel;
			pShadingModel->LoadShadingModelFromFile(InFile);

		}
		else if (strToken == "<Unknown>")
		{
			int a = 0;
		}
	}
	FBXLoad::ReadStringFromFile(InFile, strToken); // </Materials>
}

CModelInfo CGameObject::LoadModelInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& model_file_name)
{
	std::string token;

	CGameObject* frame_root = new CGameObject();

	//Heirarchy
	std::ifstream file(model_file_name, std::ios::binary);

	FBXLoad::ReadStringFromFile(file, token);

	int frames = 0;
	while (token != "</Hierarchy>")
	{
		FBXLoad::ReadStringFromFile(file, token);
		if (token == "<Frame>:")
		{
			CGameObject* frame;
			frame = CGameObject::LoadHeirarchyFromFile(pd3dDevice, pd3dCommandList, file, frames, frame_root);

			frame_root->set_child(frame);
			//if(!frame_root) 
			//	frame_root = frame;
			//else
			//	frame_root->set_sibling(frame);
		}
	}

	//Animations
	FBXLoad::ReadStringFromFile(file, token);

	CAnimationController* animation_controller = NULL;

	if (token == "<Animation>")
	{
		animation_controller = CGameObject::LoadAnimationFromFile(file, frame_root);

		FBXLoad::ReadStringFromFile(file, token);
	}

	frame_root->PrepareSkinning(pd3dDevice, pd3dCommandList, frame_root);

	CModelInfo rvalue;

	rvalue.heirarchy_root = frame_root;
	if(animation_controller)
		rvalue.animation_controller.reset(animation_controller);

	return rvalue;
}

CAnimationController* CGameObject::LoadAnimationFromFile(std::ifstream& file, CGameObject* root_object)
{
	CAnimationController* rvalue = new CAnimationController;

	int animation_count = rvalue->LoadAnimationFromFile(file);

	if (animation_count == 0)
	{
		delete rvalue;
		return NULL;
	}

	//rvalue->SetFrameCaches(root_object);

	return rvalue;
}

CGameObject* CGameObject::LoadHeirarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	std::ifstream& InFile, int& nFrames, CGameObject* root_object)
{
	CGameObject* rvalue = new CGameObject;

	std::string strToken;

	nFrames++;
	FBXLoad::ReadFromFile<int>(InFile);

	FBXLoad::ReadStringFromFile(InFile, rvalue->m_strFrameName);

	while (strToken != "</Frame>")
	{
		FBXLoad::ReadStringFromFile(InFile, strToken);
		if (strToken == "<Transform>:")
		{
			rvalue->to_parent_matrix_ = FBXLoad::ReadFromFile<XMFLOAT4X4>(InFile);

			rvalue->m_xmf3Scale = FBXLoad::ReadFromFile<XMFLOAT3>(InFile);
			rvalue->m_xmf3Rotation = FBXLoad::ReadFromFile<XMFLOAT3>(InFile);
			rvalue->m_xmf3Translation = FBXLoad::ReadFromFile<XMFLOAT3>(InFile);
		}
		else if (strToken == "<SkinDeformations>:")
		{
			CSkinMesh* pSkinMesh = new CSkinMesh();
			pSkinMesh->LoadSkinMeshFromFile(pd3dDevice, pd3dCommandList, InFile);

			FBXLoad::ReadStringFromFile(InFile, strToken);

			pSkinMesh->SetType(1);

			if (strToken == "<Mesh>:") pSkinMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, InFile);

			root_object->SetMesh(pSkinMesh);
		}
		else if (strToken == "<Mesh>:")
		{
			CMesh* pMesh = new CMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, InFile);
			root_object->SetMesh(pMesh);
		}
		else if (strToken == "<Materials>:")
		{
			rvalue->LoadMaterialFromFile(pd3dDevice, pd3dCommandList, InFile, root_object);
		}
		else if (strToken == "<Children>:")
		{
			int nChildren = FBXLoad::ReadFromFile<int>(InFile);
			for (int i = 0; i < nChildren; ++i)
			{
				FBXLoad::ReadStringFromFile(InFile, strToken);
				if (strToken == "<Frame>:")
				{
					CGameObject* pChild = CGameObject::LoadHeirarchyFromFile(pd3dDevice, pd3dCommandList, InFile, nFrames, root_object);
					if (pChild) rvalue->set_child(pChild);
				}
			}
		}
	}

	return rvalue;
}

// TODO: 오브젝트의 로테이트 다음에 구현
void CGameObject::UpdateLookVector(const XMFLOAT3& look)
{
	to_parent_matrix_._32 += look.y;
	XMMATRIX P = XMMatrixIdentity(), Y = XMMatrixIdentity(), R = XMMatrixIdentity();
	XMFLOAT3 x_axis = XMFLOAT3(1.f, 0.f, 0.f);
	XMFLOAT3 y_axis = XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3 z_axis = XMFLOAT3(0.f, 0.f, 1.f);

	if (false)
	{
		P = XMMatrixRotationAxis(XMLoadFloat3(&x_axis), XMConvertToRadians(look.x));
	}
	if (true)
	{
		Y = XMMatrixRotationAxis(XMLoadFloat3(&y_axis), XMConvertToRadians(look.y));
	}
	if (true)
	{
		R = XMMatrixRotationAxis(XMLoadFloat3(&z_axis), XMConvertToRadians(look.z));
	}

	XMMATRIX rotation_matrix = XMMatrixMultiply(XMMatrixMultiply(P, Y), R);
	set_look_vector(Vector3::TransformNormal(z_axis, rotation_matrix));
	set_right_vector(Vector3::CrossProduct(y_axis, look_vector(), true));
	set_up_vector(Vector3::CrossProduct(look_vector(), right_vector(), true));
}


//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||| <CHeightMapTerrain> |||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh* pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pHeightMapGridMesh);
	
	CMaterial* pTerrainMaterial = new CMaterial(2);

	pTerrainMaterial->AddTexturePropertyFromDDSFile(pd3dDevice, pd3dCommandList, "../Resource/Terrain/stones.dds", TextureType::RESOURCE_TEXTURE2D, 3);
	pTerrainMaterial->AddTexturePropertyFromDDSFile(pd3dDevice, pd3dCommandList, "../Resource/Terrain/Detail_Texture_7.dds", TextureType::RESOURCE_TEXTURE2D, 6);
	//pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"../Resource/Terrain/HeightMap(Alpha).dds", TextureType::RESOURCE_TEXTURE2D, 2);

	pTerrainMaterial->SetShader((int)ShaderNum::Terrain);

	SetMaterial(0, pTerrainMaterial);

	SetShader((int)ShaderNum::Terrain);

}

CHeightMapTerrain::~CHeightMapTerrain()
{
}

float CHeightMapTerrain::GetHeight(float x, float z, bool bReverseQuad)
{ 
	return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad));
} 

XMFLOAT3 CHeightMapTerrain::GetNormal(float x, float z)
{ 
	return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)));
}

int CHeightMapTerrain::GetRawImageWidth()
{ 
	return(m_pHeightMapImage->GetHeightMapWidth());
}

int CHeightMapTerrain::GetRawImageLength()
{ 
	return(m_pHeightMapImage->GetHeightMapLength());
}

CRootObject::CRootObject() : CGameObject()
{
	skinning_bone_transforms_.reserve(0);
	mapped_skinning_bone_transforms_.reserve(0);
	obb_list_.clear();
}

CRootObject::CRootObject(const CModelInfo& model) : CRootObject()
{
	model.heirarchy_root->AddRef();
	set_child(model.heirarchy_root);

	if (model.animation_controller)
	{
		animation_controller_ = new CAnimationController(*model.animation_controller);

		animation_controller_->SetFrameCaches(this);

		animation_controller_->EnableTrack(0);
	}
}

CRootObject::CRootObject(const CRootObject& other)
{
	other.child_->AddRef();
	set_child(other.child_);

	if (other.animation_controller_)
	{
		animation_controller_ = new CAnimationController(*other.animation_controller_);

		animation_controller_->SetFrameCaches(this);

		animation_controller_->EnableTrack(0);
	}

	for (auto& p : other.obb_list_)
	{
		obb_list_.push_back(new CObbComponent(*p));
	}
}

CRootObject::~CRootObject()
{
	for (auto& p : skinning_bone_transforms_) 
	{
		p->Unmap(0, NULL); 
		p->Release();
	}
	for (auto& p : obb_list_)
		delete p;
}

void CRootObject::CreateBoneTransformMatrix(ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	int mesh_count = child_->meshes_count();
	skinning_bone_transforms_.resize(mesh_count);
	mapped_skinning_bone_transforms_.resize(mesh_count);

	for (int i = 0; i < mesh_count; ++i)
	{
		UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
		skinning_bone_transforms_[i] = ::CreateBufferResource(device, command_list, NULL, ncbElementBytes);
		skinning_bone_transforms_[i]->Map(0, NULL, (void**)&mapped_skinning_bone_transforms_[i]);
	}
}

void CRootObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

void CRootObject::Rotate(float pitch, float yaw, float roll)
{
	XMMATRIX R = XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	to_parent_matrix_ = Matrix4x4::Multiply(R, to_parent_matrix_);
}

void CRootObject::Animate(float fTimeElapsed)
{
	if (animation_controller_)
	{
		ResetAnimatedSRT();
		animation_controller_->Animate(fTimeElapsed, this);
	}

	CGameObject::Animate(fTimeElapsed);

	if (parent_)
		UpdateTransform(&parent_->GetWorldMatrix());
	else
		UpdateTransform(NULL);

	for (auto& p : obb_list_)
		p->Update();
}

void CRootObject::AddObb(const BoundingBox& aabb, CGameObject* parent_socket)
{
	obb_list_.emplace_back(new CObbComponent(this, aabb, parent_socket));
}

void CRootObject::OffAllObb()
{
	for (auto& p : obb_list_)
		p->Off();
}

void CRootObject::OnAllObb()
{
	for (auto& p : obb_list_)
		p->On();
}

bool CRootObject::CollisionCheck(CRootObject* a, CRootObject* b, CObbComponent& a_obb, CObbComponent& b_obb)
{
	for (auto& p : a->obb_list_)
	{
		for (auto& other : b->obb_list_)
		{
			if (!p->is_active() || !other->is_active()) continue;
			if (p->Intersects(other->animated_obb()))
			{
				a_obb = *p;
				b_obb = *other;
				return true;
			}
		}
	}
	return false;
}

void CRootObject::CreateCollisionCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	for (auto& obb : obb_list_)
	{
		obb->CreateDebugCubeMesh(device, command_list);
	}
}

void CRootObject::RenderObbList(ID3D12GraphicsCommandList* command_list)
{
	for (auto& obb : obb_list_)
		obb->Render(command_list);
}



void CRootObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int shader_num)
{
	if (!is_visible_) return;

	UpdateShaderVariables(pd3dCommandList);

	//child의 메쉬들을 렌더한다. 계층구조의 root가 이 객체의 child이기 때문이다.
	CMesh** meshes = child_->meshes();
	int meshes_count = child_->meshes_count();

	for (int i = 0; i < meshes_count; ++i)
	{
		if (i < skinning_bone_transforms_.size())
		{
			D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = skinning_bone_transforms_[i]->GetGPUVirtualAddress();
			pd3dCommandList->SetGraphicsRootConstantBufferView(5, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms

			int bone_count = ((CSkinMesh*)meshes[i])->bone_count();
			CGameObject** bone_frame_caches = ((CSkinMesh*)meshes[i])->bone_frame_caches();
			for (int j = 0; j < bone_count; j++)
			{
				XMStoreFloat4x4(&mapped_skinning_bone_transforms_[i][j], XMMatrixTranspose(XMLoadFloat4x4(&bone_frame_caches[j]->GetWorldMatrix())));
			}
		}

		meshes[i]->Render(pd3dCommandList);
	}

	//child_->Render(pd3dCommandList, pCamera, shader_num);
}

void CSocket::Animate(float elapsed_time)
{
	UpdateTransform(&parent_->GetWorldMatrix());
	if (sibling_) sibling_->Animate(elapsed_time);
	if (child_) child_->Animate(elapsed_time);

}
