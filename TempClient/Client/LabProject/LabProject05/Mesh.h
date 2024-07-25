//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once

class CGameObject;
class CMaterial;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CVertex
{
protected:
    XMFLOAT3						m_xmf3Position;	

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }

	void SetPosition(const XMFLOAT3& xmf3Position) { m_xmf3Position = xmf3Position; }
};

class CDiffusedVertex : public CVertex
{
protected:
    XMFLOAT4						m_xmf4Diffuse;		

public:
	CDiffusedVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); }
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse; }
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)) { m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse; }
	~CDiffusedVertex() { }

	void SetDiffuse(const XMFLOAT4& xmf3Diffuse) { m_xmf4Diffuse = xmf3Diffuse; }

};

class CIlluminatedVertex : public CVertex
{
protected:
	XMFLOAT3						m_xmf3Normal;

public:
	CIlluminatedVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CIlluminatedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f)) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf3Normal = xmf3Normal; }
	CIlluminatedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f)) { m_xmf3Position = xmf3Position; m_xmf3Normal = xmf3Normal; }
	~CIlluminatedVertex() { }
};

class CDiffusedTexturedVertex : public CDiffusedVertex
{
public:
	XMFLOAT2						m_xmf2TexCoord;

public:
	CDiffusedTexturedVertex()
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f);
	}

	CDiffusedTexturedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord)
	{
		m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse; m_xmf2TexCoord = xmf2TexCoord;
	}

	CDiffusedTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT2 xmf2TexCoord = XMFLOAT2(0.0f, 0.0f))
	{
		m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse; m_xmf2TexCoord = xmf2TexCoord;
	}

	~CDiffusedTexturedVertex() { }
};

class CDiffused2TexturedVertex : public CDiffusedVertex
{
public:
	XMFLOAT2						m_xmf2TexCoord0;
	XMFLOAT2						m_xmf2TexCoord1;

public:
	CDiffused2TexturedVertex()
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord0 = XMFLOAT2(0.0f, 0.0f);
		m_xmf2TexCoord1 = XMFLOAT2(0.0f, 0.0f);
	}

	CDiffused2TexturedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord0, XMFLOAT2 xmf2TexCoord1)
	{
		m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse; m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1;
	}

	CDiffused2TexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT2 xmf2TexCoord0 = XMFLOAT2(0.0f, 0.0f), XMFLOAT2 xmf2TexCoord1 = XMFLOAT2(0.0f, 0.0f))
	{
		m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse; m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1;
	}

	~CDiffused2TexturedVertex() { }

	void SetUV(const XMFLOAT2& xmf2TexCoord0, const XMFLOAT2& xmf2TexCoord1){ m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1; }
};


//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||| <CMesh> ||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

class CMesh
{
public:
	CMesh() {}
    CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
    virtual ~CMesh();

private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

	void ReleaseUploadBuffers();

	void SetType(const int& ntype) { m_nType = ntype; }
	int GetType() { return m_nType; }

protected:
	CMaterial* material_ = NULL;

	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;

	ID3D12Resource* m_pd3dUVBuffer = NULL;
	ID3D12Resource* m_pd3dUVUploadBuffer = NULL;

	ID3D12Resource** m_ppd3dIndexBuffers = NULL;
	ID3D12Resource** m_ppd3dIndexUploadBuffers = NULL;

	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dUVBufferView;
	D3D12_INDEX_BUFFER_VIEW*		m_pd3dIndexBufferViews;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_d3dInputBufferViews;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;

	int*							m_pnIndices = 0;
	UINT							m_nStartIndex = 0;
	int								m_nBaseVertex = 0;

	//Mesh로드 관련
	std::string m_strMeshName;
	int m_nSubMeshes = 0;

	int m_nType = 0; // 0: 일반 메쉬, 1: 스킨메쉬

public:
	void set_material(CMaterial* value) { material_ = value; }

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {}

	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& pInFile);

	virtual void CreateInputBufferView();
};

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||| <CSkinMesh> ||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

#define SKINNED_ANIMATION_BONES 128
#define BONE_PER_VERTEX 4

class CSkinMesh : public CMesh
{
public:
	CSkinMesh();
	~CSkinMesh() ;

	//getter
	int bone_count() const { return m_nBones; }
	CGameObject** bone_frame_caches() { return m_BoneFrameCaches.data(); }

	void LoadSkinMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& InFile);

	void SetBoneFrameCaches(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pRootObject);

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseUploadBuffers();

	virtual void CreateInputBufferView();

private:
	std::vector<std::string> m_BoneNames;

	ID3D12Resource* bone_offset_buffer_ = NULL;
	ID3D12Resource* bone_offset_upload_buffer_ = NULL;

	ID3D12Resource* bone_index_buffer_ = NULL;
	ID3D12Resource* bone_index_upload_buffer_ = NULL;
	D3D12_VERTEX_BUFFER_VIEW bone_index_buffer_view_;

	ID3D12Resource* bone_weight_buffer_ = NULL;
	ID3D12Resource* bone_weight_upload_buffer_ = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_BoneWeightBufferView;

	std::vector<CGameObject*> m_BoneFrameCaches;

	ID3D12Resource* skinning_bone_transforms_ = NULL;
	XMFLOAT4X4* mapped_skinning_bone_transforms_;

	int m_nBones;
};

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||||| <CHeightMapImage> |||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

class CHeightMapImage
{
private:
	BYTE* m_pHeightMapPixels;

	int m_nWidth;
	int m_nLength;

	XMFLOAT3 m_xmf3Scale; // 픽셀 크기

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage();

	float GetHeight(float fx, float fz, bool bReverseQuad);
	XMFLOAT3 GetHeightMapNormal(int nx, int nz);

	BYTE* GetHeightMapPixels() { return m_pHeightMapPixels; }

	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	int GetHeightMapWidth() { return m_nWidth; }
	int GetHeightMapLength() { return m_nLength; }

};



//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//|||||||||||||||||||||||||||||||||||||||||||||||| <CHeightMapGridMesh> ||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

class CHeightMapGridMesh : public CMesh
{
protected:
	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void* pContext = NULL);

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void* pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void* pContext);
};

class CSkyBoxMesh : public CMesh
{
public:
	CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();
};

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const XMFLOAT3& min, const XMFLOAT3& max);
	
};
