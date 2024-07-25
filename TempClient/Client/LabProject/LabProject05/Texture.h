#pragma once

#define TEXTURE_FILE_ROOT "../Resource/Model/Texture/"

enum class TextureType
{
	NONE = 0,
	RESOURCE_TEXTURE2D = 0x01,
	RESOURCE_TEXTURE2D_ARRAY = 0x02,
	RESOURCE_TEXTURE2DARRAY = 0x03,
	RESOURCE_TEXTURE_CUBE = 0x04,
	RESOURCE_BUFFER = 0x05
};

enum class TextureLoadType
{
	File = 0,
	Procedural = 1
};

enum class TextureMappingType
{
	Null = 0,
	Planar = 1,
	Spherical = 2,
	Cylindrical = 3,
	Box = 4,
	Face = 5,
	UV = 6,
	Environment = 7
};

enum class TextureBlendMode
{
	None = -1,
	Translucent = 0,
	Additive = 1,
	Modulate = 2,
	Modulate2,
	Over,
	Normal,
	Dissolve,
	Darken,
	ColorBurn,
	LinearBurn,
	DarkerColor,
	Lighten,
	Screen,
	ColorDodge,
	LinearDodge,
	LighterColor,
	SoftLight,
	HardLight,
	VividLight,
	LinearLight,
	PinLight,
	HardMix,
	Difference,
	Exclusion,
	Substract,
	Divide,
	Hue,
	Saturation,
	Color,
	Luminosity,
	Overlay
};

enum class TextureMaterialUseType
{
	None = -1,
	Model = 0,
	Default
};

enum class TextureUseType
{
	Standard = 0,
	ShadowMap,
	LightMap,
	SphericalReflexionMap,
	SphereReflexionMap,
	BumpNormalMap
};

class CDescriptorManager;

class CTexture
{
public:
	CTexture();
	CTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile);
	virtual ~CTexture();

private:
	//Fbx Texture Info
	std::string m_strTextureName;

	TextureLoadType m_LoadType;

	std::string m_strTextureFileName;

	XMFLOAT2 m_UVScale;
	XMFLOAT2 m_UVTranslation;
	bool m_UVSwap;
	XMFLOAT3 m_UVWRotation;

	int m_AlphaSource;

	XMINT4 m_CroppingBox; //Left, Top, Right, Bottom

	TextureMappingType m_MappingType;
	TextureBlendMode m_BlendMode;

	float m_DefaultAlpha;

	TextureMaterialUseType m_MaterialUseType;
	TextureUseType m_TextureUseType;

	// Render ฐüทร
	TextureType m_ResourceType;

	ID3D12Resource* m_pd3dTexture = NULL;
	ID3D12Resource* m_pd3dTextureUploadBuffer = NULL;

	UINT m_RootParameterIndex;

	DXGI_FORMAT m_BufferFormat;
	UINT m_BufferElements;

	D3D12_GPU_DESCRIPTOR_HANDLE m_SrvGpuDescriptorHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE m_SamplerGpuDescriptorHandle;

public:
	void SetSampler(D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile);

	void LoadTextureFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex);
	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex);

	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, TextureType nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

	void SetRootParameterIndex(UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() { return(m_SrvGpuDescriptorHandle); }

	std::string& GetTextureName() { return(m_strTextureName); }
	ID3D12Resource* GetResource() { return(m_pd3dTexture); }
	int GetRootParameter() { return(m_RootParameterIndex); }

	TextureType GetTextureType() { return(m_ResourceType); }
	DXGI_FORMAT GetBufferFormat() { return(m_BufferFormat); }
	int GetBufferElements() { return(m_BufferElements); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc();

	void ReleaseUploadBuffer();

	void CreateShaderResourceView(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager);
};
