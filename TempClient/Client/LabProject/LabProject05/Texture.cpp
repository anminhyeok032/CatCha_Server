#include "stdafx.h"
#include "Texture.h"
#include "DescriptorManager.h"

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||| <CTexture> ||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CTexture::CTexture()
{

}

CTexture::CTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile)
{
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken);

	if (strToken == "<Texture>:")
		LoadTextureFromFile(pd3dDevice, pd3dCommandList, InFile);
}

CTexture::~CTexture()
{
	if (m_pd3dTexture)
		m_pd3dTexture->Release();
	if (m_pd3dTextureUploadBuffer) 
		m_pd3dTextureUploadBuffer->Release();
}

void CTexture::SetRootParameterIndex(UINT nRootParameterIndex)
{
	m_RootParameterIndex = nRootParameterIndex;
}

void CTexture::SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_SrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_SamplerGpuDescriptorHandle = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//TODO: 텍스처가 중복으로 set되지 않게 수정 혹은 텍스처 set에 관한 순서 최적화 필요
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_RootParameterIndex, m_SrvGpuDescriptorHandle);
}

void CTexture::ReleaseShaderVariables()
{

}

void CTexture::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile)
{
	FBXLoad::ReadFromFile<int>(InFile); //index

	std::string strToken;

	FBXLoad::ReadStringFromFile(InFile, strToken);
	m_strTextureName = strToken;

	m_LoadType = (TextureLoadType)FBXLoad::ReadFromFile<int>(InFile);

	FBXLoad::ReadStringFromFile(InFile, strToken);
	m_strTextureFileName = strToken;
	m_strTextureFileName = TEXTURE_FILE_ROOT + m_strTextureFileName.substr(0, m_strTextureFileName.size());
	
	//TODO: 루트파라미터 인덱스 관련 상수 정리
	LoadTextureFromWICFile(pd3dDevice, pd3dCommandList, m_strTextureFileName, TextureType::RESOURCE_TEXTURE2D, 7); 

	m_UVScale.x = FBXLoad::ReadFromFile<float>(InFile);
	m_UVScale.y = FBXLoad::ReadFromFile<float>(InFile);

	m_UVTranslation.x = FBXLoad::ReadFromFile<float>(InFile);
	m_UVTranslation.y = FBXLoad::ReadFromFile<float>(InFile);

	m_UVSwap = (bool)FBXLoad::ReadFromFile<int>(InFile);

	m_UVWRotation.x = FBXLoad::ReadFromFile<float>(InFile);
	m_UVWRotation.y = FBXLoad::ReadFromFile<float>(InFile);
	m_UVWRotation.z = FBXLoad::ReadFromFile<float>(InFile);

	m_AlphaSource = FBXLoad::ReadFromFile<int>(InFile);

	m_CroppingBox.x = FBXLoad::ReadFromFile<int>(InFile); // left
	m_CroppingBox.y = FBXLoad::ReadFromFile<int>(InFile); // top
	m_CroppingBox.z = FBXLoad::ReadFromFile<int>(InFile); // right
	m_CroppingBox.w = FBXLoad::ReadFromFile<int>(InFile); // bottom

	m_MappingType = (TextureMappingType)FBXLoad::ReadFromFile<int>(InFile);

	m_BlendMode = (TextureBlendMode)FBXLoad::ReadFromFile<int>(InFile);

	m_DefaultAlpha = FBXLoad::ReadFromFile<float>(InFile);

	m_MaterialUseType = (TextureMaterialUseType)FBXLoad::ReadFromFile<int>(InFile);

	m_TextureUseType = (TextureUseType)FBXLoad::ReadFromFile<int>(InFile);
}

void CTexture::ReleaseUploadBuffer()
{
	m_pd3dTextureUploadBuffer->Release();
	m_pd3dTextureUploadBuffer = NULL;
}

void CTexture::CreateShaderResourceView(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc();
	pd3dDevice->CreateShaderResourceView(m_pd3dTexture, &d3dShaderResourceViewDesc, pDescriptorManager->GetSrvNextCPUHandle());

	m_SrvGpuDescriptorHandle = pDescriptorManager->GetSrvNextGPUHandle();

	pDescriptorManager->AddSrvIndex();
}

void CTexture::LoadTextureFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex)
{
	m_ResourceType = nResourceType;
	m_RootParameterIndex = nRootParameterIndex;
	m_strTextureFileName = strFileName;

	std::wstring wstrFileName;
	wstrFileName.assign(strFileName.begin(), strFileName.end());
	m_pd3dTexture = ::CreateTextureResourceFromWICFile(pd3dDevice, pd3dCommandList, wstrFileName.c_str(), &m_pd3dTextureUploadBuffer, D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}


void CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex)
{
	m_ResourceType = nResourceType;
	m_RootParameterIndex = nRootParameterIndex;
	m_strTextureFileName = strFileName;

	std::wstring wstrFileName;
	wstrFileName.assign(strFileName.begin(), strFileName.end());
	m_pd3dTexture = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, wstrFileName.c_str(), &m_pd3dTextureUploadBuffer, D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

void CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex)
{
	m_ResourceType = TextureType::RESOURCE_BUFFER;
	m_BufferFormat = ndxgiFormat;
	m_BufferElements = nElements;
	m_pd3dTexture = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, &m_pd3dTextureUploadBuffer);
}

ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, TextureType nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	m_ResourceType = nResourceType;
	m_pd3dTexture = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, nElements, nMipLevels, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_pd3dTexture);
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc()
{
	D3D12_RESOURCE_DESC d3dResourceDesc = m_pd3dTexture->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (m_ResourceType)
	{
	case TextureType::RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case TextureType::RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case TextureType::RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case TextureType::RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case TextureType::RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_BufferFormat;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_BufferElements;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}