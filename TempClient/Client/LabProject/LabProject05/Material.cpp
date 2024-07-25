#include "stdafx.h"
#include "Material.h"
#include "DescriptorManager.h"

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||| <CTextureProperty> ||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CTextureProperty::CTextureProperty(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex)
{
	m_Textures.emplace_back();

	if (strFileName.substr(strFileName.length() - 3, 3) == "dds")
		m_Textures.back().LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, strFileName, nResourceType, nRootParameterIndex);
	else
		m_Textures.back().LoadTextureFromWICFile(pd3dDevice, pd3dCommandList, strFileName, nResourceType, nRootParameterIndex);
}

CTextureProperty::CTextureProperty(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile, const std::string& strName) : m_PropertyName(strName)
{
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken);
	if (strToken == "<Textures>:")
		LoadTexturePropertyFromFile(pd3dDevice, pd3dCommandList, InFile);
}

void CTextureProperty::LoadTexturePropertyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile)
{
	int nTextures = FBXLoad::ReadFromFile<int>(InFile);
	m_Textures.reserve(nTextures);
	for (int i = 0; i < nTextures; ++i)
	{
		m_Textures.emplace_back(pd3dDevice, pd3dCommandList, InFile);
	}

}

void CTextureProperty::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& Texture : m_Textures)
		Texture.UpdateShaderVariable(pd3dCommandList);
}

void CTextureProperty::ReleaseUploadBuffers()
{
	for (auto& Texture : m_Textures)
		Texture.ReleaseUploadBuffer();
}

void CTextureProperty::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager)
{
	if (m_Textures.size())
	{
		for (auto& texture : m_Textures)
		{
			texture.CreateShaderResourceView(pd3dDevice, pDescriptorManager);
		}
	}
}


//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||| <CPhongModel> ||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void CPhongModel::LoadShadingModelFromFile(std::ifstream& InFile)
{
	m_Ambient.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Ambient.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Ambient.z = FBXLoad::ReadFromFile<float>(InFile);

	m_Diffuse.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Diffuse.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Diffuse.z = FBXLoad::ReadFromFile<float>(InFile);

	m_Specular.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Specular.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Specular.z = FBXLoad::ReadFromFile<float>(InFile);

	m_Emissive.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Emissive.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Emissive.z = FBXLoad::ReadFromFile<float>(InFile);

	m_TransparencyFactor = FBXLoad::ReadFromFile<float>(InFile);
	m_Shininess = FBXLoad::ReadFromFile<float>(InFile);
	m_ReflectionFactor = FBXLoad::ReadFromFile<float>(InFile);

}



//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||| <CLambertModel> |||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void CLambertModel::LoadShadingModelFromFile(std::ifstream& InFile)
{
	m_Ambient.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Ambient.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Ambient.z = FBXLoad::ReadFromFile<float>(InFile);

	m_Diffuse.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Diffuse.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Diffuse.z = FBXLoad::ReadFromFile<float>(InFile);

	m_Emissive.x = FBXLoad::ReadFromFile<float>(InFile);
	m_Emissive.y = FBXLoad::ReadFromFile<float>(InFile);
	m_Emissive.z = FBXLoad::ReadFromFile<float>(InFile);

	m_TransparencyFactor = FBXLoad::ReadFromFile<float>(InFile);
}

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||||| <CMaterial> |||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CMaterial::CMaterial()
{
}

CMaterial::CMaterial(const int& nProperty)
{
	m_TextureProperties.reserve(nProperty);
}

CMaterial::~CMaterial()
{
	if (m_pShadingModel)
		delete m_pShadingModel;
}

void CMaterial::ReleaseUploadBuffers()
{
	for (auto& Property : m_TextureProperties)
		Property.ReleaseUploadBuffers();
}

void CMaterial::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CDescriptorManager* pDescriptorManager)
{
	for (auto& Property : m_TextureProperties)
		Property.CreateShaderResourceViews(pd3dDevice, pDescriptorManager);
}

void CMaterial::LoadTexturePropertiesFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile)
{
	std::string strToken;

	int nProperties = FBXLoad::ReadFromFile<int>(InFile);

	m_TextureProperties.reserve(nProperties);

	for (int j = 0; j < nProperties; ++j)
	{
		FBXLoad::ReadStringFromFile(InFile, strToken); // <Property>:
		FBXLoad::ReadFromFile<int>(InFile); // index
		FBXLoad::ReadStringFromFile(InFile, strToken); //TextureChannelName
		FBXLoad::ReadStringFromFile(InFile, strToken); //strPropertyName
		if (strToken != "Null")
		{
			m_TextureProperties.emplace_back(pd3dDevice, pd3dCommandList, InFile, strToken);
		}
	}
}

void CMaterial::AddTexturePropertyFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const std::string& strFileName, TextureType nResourceType, UINT nRootParameterIndex)
{
	m_TextureProperties.emplace_back(pd3dDevice, pd3dCommandList, strFileName, nResourceType, nRootParameterIndex);
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& Property : m_TextureProperties)
		Property.UpdateShaderVariables(pd3dCommandList);
}