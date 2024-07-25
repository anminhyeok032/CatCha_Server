#pragma once

#include "Texture.h"

// 재질이 가지는 특성
class CTextureProperty
{
private:
	std::string m_PropertyName;
	std::vector<CTexture> m_Textures; // 각 특성을 표현하기위한 텍스처들

public:
	CTextureProperty() {}
	// dds 파일로 텍스처 직접 추가
	CTextureProperty(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex);
	// fbx 파일을 익스포트한 bin 파일 로드
	CTextureProperty(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile, const std::string& strName);
	~CTextureProperty() {}

	void LoadTexturePropertyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseUploadBuffers();

	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager);
};

//TODO: ShadingModel이 어떤의미인지 어떻게 사용되는 지 공부
class CShadingModel
{
private:
	std::string m_strName;

public:
	virtual void LoadShadingModelFromFile(std::ifstream& InFile) {}
};

class CPhongModel : public CShadingModel
{
private:
	XMFLOAT3 m_Ambient;
	XMFLOAT3 m_Diffuse;
	XMFLOAT3 m_Specular;
	XMFLOAT3 m_Emissive;

	float m_TransparencyFactor;
	float m_Shininess;
	float m_ReflectionFactor;

public:
	virtual void LoadShadingModelFromFile(std::ifstream& InFile);


};

class CLambertModel : public CShadingModel
{
private:
	XMFLOAT3 m_Ambient;
	XMFLOAT3 m_Diffuse;
	XMFLOAT3 m_Emissive;

	float m_TransparencyFactor;

public:
	virtual void LoadShadingModelFromFile(std::ifstream& InFile);


};

class CMaterial
{
public:
	CMaterial();
	CMaterial(const int& nProperty);
	virtual ~CMaterial();

private:
	int	m_nReferences = 0;

	int m_nShader = NULL;
	std::vector<CTextureProperty> m_TextureProperties;

	CShadingModel* m_pShadingModel = NULL;

	std::string m_strName;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	void SetShader(const int& nShader) { m_nShader = nShader; }

	void SetName(const std::string& strName) { m_strName = strName; }

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();

	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager);

	void LoadTexturePropertiesFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile);

	void AddTexturePropertyFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex);
};