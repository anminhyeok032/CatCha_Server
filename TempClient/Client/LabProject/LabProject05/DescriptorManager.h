#pragma once
class CDescriptorManager
{
private:
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;

	UINT m_nDescriptors = 0;

	// 힙 생성시 한번만 저장
	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	// NextHadle == StartHandle + Index * IncrementSize
	UINT m_nCbvIndex = 0;
	UINT m_nSrvIndex = 0;

public:
	~CDescriptorManager() { m_pd3dCbvSrvDescriptorHeap->Release(); }

	ID3D12DescriptorHeap*& GetDescriptorHeap() { return m_pd3dCbvSrvDescriptorHeap; }
	void SetDesriptorHeap(ID3D12DescriptorHeap* p) { m_pd3dCbvSrvDescriptorHeap = p; }

	UINT GetDescriptors() const { return m_nDescriptors; }
	void SetDescriptors(const UINT& nValue) { m_nDescriptors = nValue; }

	void Initialization(UINT nCbvs);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCbvNextCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvNextGPUHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvNextCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvNextGPUHandle() const;

	void AddCbvIndex() { m_nCbvIndex++; }
	void AddSrvIndex() { m_nSrvIndex++; }

};

