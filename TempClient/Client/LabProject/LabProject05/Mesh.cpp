#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"
#include "Material.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_nReferences = 0;

	m_pd3dVertexBuffer = NULL;
	m_pd3dVertexUploadBuffer = NULL;

	m_ppd3dIndexBuffers = NULL;
	m_ppd3dIndexUploadBuffers = NULL;

	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nSlot = 0;
	m_nVertices = 0;
	m_nStride = 0;
	m_nOffset = 0;
}

CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_ppd3dIndexBuffers) for(int i = 0; i < m_nSubMeshes; ++i) m_ppd3dIndexBuffers[i]->Release();
	delete[] m_ppd3dIndexBuffers;

	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_ppd3dIndexUploadBuffers) for (int i = 0; i < m_nSubMeshes; ++i) m_ppd3dIndexUploadBuffers[i]->Release();
	delete[] m_ppd3dIndexUploadBuffers;

	if (m_pd3dUVBuffer) m_pd3dUVBuffer->Release();
}

void CMesh::AddRef() 
{ 
	m_nReferences++; 
}

void CMesh::Release() 
{ 
	m_nReferences--; 
	if (m_nReferences <= 0) delete this;
}

void CMesh::ReleaseUploadBuffers() 
{
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_ppd3dIndexUploadBuffers) for (int i = 0; i < m_nSubMeshes; ++i) m_ppd3dIndexUploadBuffers[i]->Release();
	delete[] m_ppd3dIndexUploadBuffers;

	if (m_pd3dUVUploadBuffer) 
		m_pd3dUVUploadBuffer->Release();

	m_pd3dVertexUploadBuffer = NULL;
	m_ppd3dIndexUploadBuffers = NULL;
	m_pd3dUVUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if(material_) 
		material_->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	OnPreRender(pd3dCommandList);

	if (m_ppd3dIndexBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; ++i)
		{
			if (!m_pnIndices[i]) continue;
			pd3dCommandList->IASetIndexBuffer(&m_pd3dIndexBufferViews[i]);
			pd3dCommandList->DrawIndexedInstanced(m_pnIndices[i], 1, 0, 0, 0);
		}
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);

}

void CMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile)
{
	FBXLoad::ReadStringFromFile(InFile, m_strMeshName);

	std::string strToken;


	while (strToken != "</Mesh>")
	{
		FBXLoad::ReadStringFromFile(InFile, strToken);
		if (strToken == "<ControlPoints>:")
		{
			m_nVertices = FBXLoad::ReadFromFile<UINT>(InFile);

			XMFLOAT3* pVertices = new XMFLOAT3[m_nVertices];

			FBXLoad::ReadFromFile<XMFLOAT3>(InFile, pVertices, m_nVertices);

			m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, 
				sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

			m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
			m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

			delete[] pVertices;
		}
		else if (strToken == "<UVs>:")
		{
			FBXLoad::ReadFromFile<int>(InFile); // nControlPoints
			FBXLoad::ReadFromFile<int>(InFile); // UVs Per Polygon's Vertex

			FBXLoad::ReadStringFromFile(InFile, strToken); // "<UV>:"

			FBXLoad::ReadFromFile<int>(InFile); //index?
			
			XMFLOAT2* pUVs = new XMFLOAT2[m_nVertices];

			FBXLoad::ReadFromFile<XMFLOAT2>(InFile, pUVs, m_nVertices);

			m_pd3dUVBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pUVs,
				sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dUVUploadBuffer);

			m_d3dUVBufferView.BufferLocation = m_pd3dUVBuffer->GetGPUVirtualAddress();
			m_d3dUVBufferView.StrideInBytes = sizeof(XMFLOAT2);
			m_d3dUVBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

		}
		else if (strToken == "<Polygons>:")
		{
			int nPolygons = FBXLoad::ReadFromFile<int>(InFile);
			
			FBXLoad::ReadStringFromFile(InFile, strToken);
			if (strToken == "<SubIndices>:")
			{
				int nIndices = FBXLoad::ReadFromFile<int>(InFile);
				m_nSubMeshes = FBXLoad::ReadFromFile<int>(InFile);

				m_pnIndices = new int[m_nSubMeshes];
				UINT** ppIndices = new UINT * [m_nSubMeshes];

				m_ppd3dIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
				m_ppd3dIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
				m_pd3dIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; ++i)
				{
					FBXLoad::ReadStringFromFile(InFile, strToken);
					if (strToken == "<SubIndex>:")
					{
						int nIndex = FBXLoad::ReadFromFile<int>(InFile);
						m_pnIndices[i] = FBXLoad::ReadFromFile<int>(InFile);
						if (!m_pnIndices[i])
						{
							m_nSubMeshes--;
							i--;
							continue;
						}
						ppIndices[i] = new UINT[m_pnIndices[i]];
						FBXLoad::ReadFromFile<UINT>(InFile, ppIndices[i], m_pnIndices[i]);

						m_ppd3dIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
							ppIndices[i], sizeof(UINT) * m_pnIndices[i], D3D12_HEAP_TYPE_DEFAULT,
							D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dIndexUploadBuffers[i]);

						m_pd3dIndexBufferViews[i].BufferLocation = m_ppd3dIndexBuffers[i]->GetGPUVirtualAddress();
						m_pd3dIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
						m_pd3dIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnIndices[i];
					}
				}

				for (int i = 0; i < m_nSubMeshes; ++i) {
					if (!m_pnIndices[i]) continue;
					delete ppIndices[i];
				}
				delete[] ppIndices;
			}
		}
	}
	CreateInputBufferView();
}

void CMesh::CreateInputBufferView()
{
	if (m_pd3dVertexBuffer)
		m_d3dInputBufferViews.emplace_back(m_d3dVertexBufferView);
	if (m_pd3dUVBuffer)
		m_d3dInputBufferViews.emplace_back(m_d3dUVBufferView);

	//TODO: 텍스처 관련 버퍼 추가(normal, bitangent, tangent)

}

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||| <CHeightMapImage> |||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);

	DWORD dwByteRead;
	::ReadFile(hFile, pHeightMapPixels, m_nWidth * m_nLength, &dwByteRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	// 지형좌표계와 픽셀좌표계가 다른점을 고려하여..
	for (int z = 0; z < m_nLength; z++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - z) * m_nWidth)] = pHeightMapPixels[x + (z * m_nWidth)];
		}
	}
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
}

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;

	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];

	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}

	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight * m_xmf3Scale.y);

}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;

	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;

	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}



//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||| <CHeightMapGridMesh> ||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList,
	int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext) : CMesh(pd3dDevice, pd3dCommandList) // pContext == HeightMapImage
{
	m_nVertices = nWidth * nLength;
	m_nStride = sizeof(CDiffused2TexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	CDiffused2TexturedVertex* pVertices = new CDiffused2TexturedVertex[m_nVertices];
	
	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			float fHeight = OnGetHeight(x, z, pContext);
			pVertices[i].SetPosition(XMFLOAT3((x * m_xmf3Scale.x), fHeight * m_xmf3Scale.y, (z * m_xmf3Scale.z)));
			pVertices[i].SetDiffuse(Vector4::Add(OnGetColor(x, z, pContext), xmf4Color));
			pVertices[i].SetUV(XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1)), 
				XMFLOAT2(float(x) / float(m_xmf3Scale.x * 0.5f), float(z) / float(m_xmf3Scale.z * 0.5f)));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, 
		sizeof(CDiffused2TexturedVertex) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(CDiffused2TexturedVertex);
	m_d3dVertexBufferView.SizeInBytes = sizeof(CDiffused2TexturedVertex) * m_nVertices;

	m_nSubMeshes = 1;
	m_pnIndices = new int[m_nSubMeshes];
	m_pnIndices[0] = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
	UINT* pnIndices = new UINT[m_pnIndices[0]];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) pnIndices[j++] = (UINT)(x + (z * nWidth));
				pnIndices[j++] = (UINT)(x + (z * nWidth));
				pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) pnIndices[j++] = (UINT)(x + (z * nWidth));
				pnIndices[j++] = (UINT)(x + (z * nWidth));
				pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_ppd3dIndexBuffers = new ID3D12Resource*;
	m_ppd3dIndexUploadBuffers = new ID3D12Resource*;
	m_ppd3dIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
		pnIndices, sizeof(UINT) * m_pnIndices[0], D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dIndexUploadBuffers[0]);

	m_pd3dIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW;
	m_pd3dIndexBufferViews->BufferLocation = m_ppd3dIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dIndexBufferViews->Format = DXGI_FORMAT_R32_UINT;
	m_pd3dIndexBufferViews->SizeInBytes = sizeof(UINT) * m_pnIndices[0];

}


float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;

	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();

	int nWidth = pHeightMapImage->GetHeightMapWidth();

	float fHeight = pHeightMapPixels[x + (z * nWidth)];

	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);

	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);

	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;

	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);

	return(xmf4Color);
}

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||||| <CSkinMesh> |||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

CSkinMesh::CSkinMesh() : CMesh()
{

}

CSkinMesh::~CSkinMesh()
{
	ReleaseUploadBuffers();

	if(bone_offset_buffer_) bone_offset_buffer_->Release();

	if (bone_index_buffer_) bone_index_buffer_->Release();

	if (bone_weight_buffer_) bone_weight_buffer_->Release();

	if (skinning_bone_transforms_) skinning_bone_transforms_->Release();

}


void CSkinMesh::LoadSkinMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile)
{
	std::string strToken;

	FBXLoad::ReadStringFromFile(InFile, strToken);

	while (strToken != "</SkinDeformations>")
	{
		if (strToken == "<BoneNames>:")
		{
			m_nBones = FBXLoad::ReadFromFile<int>(InFile);
			
			m_BoneNames.reserve(m_nBones);

			for (int i = 0; i < m_nBones; ++i)
			{
				FBXLoad::ReadStringFromFile(InFile, strToken);
				m_BoneNames.emplace_back(strToken);
			}
		}
		else if(strToken == "<BoneOffsets>:")
		{
			FBXLoad::ReadFromFile<int>(InFile); //nBones

			XMFLOAT4X4* pData = new XMFLOAT4X4[m_nBones];

			FBXLoad::ReadFromFile<XMFLOAT4X4>(InFile, pData, m_nBones);

			for(int i = 0; i < m_nBones; ++i)
			{
				XMStoreFloat4x4(&pData[i], XMMatrixTranspose(XMLoadFloat4x4(&pData[i])));
			}
			
			UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * m_nBones) + 255) & ~255); //256의 배수
			bone_offset_buffer_ = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
				pData, ncbElementBytes, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &bone_offset_upload_buffer_);

		}
		else if (strToken == "<BoneIndices>:")
		{
			int nControllPoints = FBXLoad::ReadFromFile<int>(InFile);

			XMINT4* pData = new XMINT4[nControllPoints];

			FBXLoad::ReadFromFile<XMINT4>(InFile, pData, nControllPoints);

			bone_index_buffer_ = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
				pData, sizeof(XMINT4) * nControllPoints, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &bone_index_upload_buffer_);
			
			bone_index_buffer_view_.BufferLocation = bone_index_buffer_->GetGPUVirtualAddress();
			bone_index_buffer_view_.StrideInBytes = sizeof(XMINT4);
			bone_index_buffer_view_.SizeInBytes = sizeof(XMINT4) * nControllPoints;

		}
		else if (strToken == "<BoneWeights>:")
		{
			int nControllPoints = FBXLoad::ReadFromFile<int>(InFile);

			XMFLOAT4* pData = new XMFLOAT4[nControllPoints];

			FBXLoad::ReadFromFile<XMFLOAT4>(InFile, pData, nControllPoints);

			bone_weight_buffer_ = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
				pData, sizeof(XMFLOAT4) * nControllPoints, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &bone_weight_upload_buffer_);

			m_BoneWeightBufferView.BufferLocation = bone_weight_buffer_->GetGPUVirtualAddress();
			m_BoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
			m_BoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * nControllPoints;

		}

		FBXLoad::ReadStringFromFile(InFile, strToken);

	}
}

void CSkinMesh::SetBoneFrameCaches(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pRootObject)
{
	m_BoneFrameCaches.reserve(m_nBones);

	for (int i = 0; i < m_nBones; ++i)
	{
		m_BoneFrameCaches.emplace_back(pRootObject->FindFrame(m_BoneNames[i]));
	}

	pRootObject->SetBoneFrameCaches(m_BoneFrameCaches.data(), m_nBones);


}

void CSkinMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, m_d3dInputBufferViews.size(), m_d3dInputBufferViews.data());
}

void CSkinMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (bone_offset_buffer_)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = bone_offset_buffer_->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(4, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets
	}

}

void CSkinMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (bone_offset_upload_buffer_) bone_offset_upload_buffer_->Release();
	bone_offset_upload_buffer_ = NULL;

	if (bone_index_upload_buffer_) bone_index_upload_buffer_->Release();
	bone_index_upload_buffer_ = NULL;

	if (bone_weight_upload_buffer_) bone_weight_upload_buffer_->Release();
	bone_weight_upload_buffer_ = NULL;
}

void CSkinMesh::CreateInputBufferView()
{
	CMesh::CreateInputBufferView();

	m_d3dInputBufferViews.emplace_back(bone_index_buffer_view_);
	m_d3dInputBufferViews.emplace_back(m_BoneWeightBufferView);
}


CSkyBoxMesh::CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	XMFLOAT3* positions = new XMFLOAT3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	positions[0] = XMFLOAT3(-fx, +fx, +fx);
	positions[1] = XMFLOAT3(+fx, +fx, +fx);
	positions[2] = XMFLOAT3(-fx, -fx, +fx);
	positions[3] = XMFLOAT3(-fx, -fx, +fx);
	positions[4] = XMFLOAT3(+fx, +fx, +fx);
	positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	positions[6] = XMFLOAT3(+fx, +fx, -fx);
	positions[7] = XMFLOAT3(-fx, +fx, -fx);
	positions[8] = XMFLOAT3(+fx, -fx, -fx);
	positions[9] = XMFLOAT3(+fx, -fx, -fx);
	positions[10] = XMFLOAT3(-fx, +fx, -fx);
	positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	positions[12] = XMFLOAT3(-fx, +fx, -fx);
	positions[13] = XMFLOAT3(-fx, +fx, +fx);
	positions[14] = XMFLOAT3(-fx, -fx, -fx);
	positions[15] = XMFLOAT3(-fx, -fx, -fx);
	positions[16] = XMFLOAT3(-fx, +fx, +fx);
	positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	positions[18] = XMFLOAT3(+fx, +fx, +fx);
	positions[19] = XMFLOAT3(+fx, +fx, -fx);
	positions[20] = XMFLOAT3(+fx, -fx, +fx);
	positions[21] = XMFLOAT3(+fx, -fx, +fx);
	positions[22] = XMFLOAT3(+fx, +fx, -fx);
	positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	positions[24] = XMFLOAT3(-fx, +fx, -fx);
	positions[25] = XMFLOAT3(+fx, +fx, -fx);
	positions[26] = XMFLOAT3(-fx, +fx, +fx);
	positions[27] = XMFLOAT3(-fx, +fx, +fx);
	positions[28] = XMFLOAT3(+fx, +fx, -fx);
	positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	positions[30] = XMFLOAT3(-fx, -fx, +fx);
	positions[31] = XMFLOAT3(+fx, -fx, +fx);
	positions[32] = XMFLOAT3(-fx, -fx, -fx);
	positions[33] = XMFLOAT3(-fx, -fx, -fx);
	positions[34] = XMFLOAT3(+fx, -fx, +fx);
	positions[35] = XMFLOAT3(+fx, -fx, -fx);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CSkyBoxMesh::~CSkyBoxMesh()
{
}

CCubeMesh::CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const XMFLOAT3& min, const XMFLOAT3& max) : CMesh(pd3dDevice, pd3dCommandList)
{
	//TODO: 바운딩 박스 메쉬관련 작업필요
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	XMFLOAT3* positions = new XMFLOAT3[m_nVertices];

	positions[0] = XMFLOAT3(min.x, max.y, min.z);
	positions[1] = XMFLOAT3(min.x, max.y, max.z);
	positions[2] = XMFLOAT3(max.x, max.y, max.z);
	positions[3] = XMFLOAT3(max.x, max.y, min.z);
	positions[4] = XMFLOAT3(min.x, min.y, min.z);
	positions[5] = XMFLOAT3(min.x, min.y, max.z);
	positions[6] = XMFLOAT3(max.x, min.y, max.z);
	positions[7] = XMFLOAT3(max.x, min.y, min.z);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = 1;
	m_pnIndices = new int[m_nSubMeshes];
	m_pnIndices[0] = 36;

	UINT indices[] {
		//top
		0, 1, 2,
		0, 2, 3,
		//forward
		0,3,4,
		3,7,4,
		//left
		1,0,5,
		0,4,5,
		//right
		3,6,7,
		3,2,6,
		//back
		1,5,2,
		2,5,6,
		//bottom
		5,4,6,
		6,4,7
	};

	m_ppd3dIndexBuffers = new ID3D12Resource * [1];
	m_ppd3dIndexUploadBuffers = new ID3D12Resource * [1];
	m_ppd3dIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
		indices, sizeof(UINT) * m_pnIndices[0], D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dIndexUploadBuffers[0]);

	m_pd3dIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[1];
	m_pd3dIndexBufferViews[0].BufferLocation = m_ppd3dIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnIndices[0];


}
