#include "stdafx.h"
#include "SkyBox.h"
#include "Mesh.h"
#include "Material.h"
#include "Shader.h"
#include "Camera.h"

CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CGameObject()
{
	CSkyBoxMesh* skybox_mesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 100.0f, 100.0f, 2.0f);
	SetMesh(skybox_mesh);

	CMaterial* material = new CMaterial(1);
	material->AddTexturePropertyFromDDSFile(pd3dDevice, pd3dCommandList, "../Resource/SkyBox/SkyBox(Galaxy).dds", TextureType::RESOURCE_TEXTURE_CUBE, 8);

	SetMaterial(0, material);

	SetShader((int)ShaderNum::SkyBox);
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int shader_num)
{
	set_position_vector(pCamera->GetPosition());

	UpdateTransform(NULL);

	CGameObject::Render(pd3dCommandList, pCamera, shader_num);
}
