#pragma once
#include "Object.h"

class CCamera;

class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~CSkyBox() {}

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int shader_num);
};