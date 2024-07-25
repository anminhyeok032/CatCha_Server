#include "stdafx.h"
#include "DescriptorManager.h"

void CDescriptorManager::Initialization(UINT nCbvs)
{
	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize) * nCbvs;
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize) * nCbvs;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorManager::GetCbvNextCPUHandle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE rValue = m_d3dCbvCPUDescriptorStartHandle;
	rValue.ptr += ::gnCbvSrvDescriptorIncrementSize * m_nCbvIndex;
	return rValue;
}

D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorManager::GetCbvNextGPUHandle() const
{
	D3D12_GPU_DESCRIPTOR_HANDLE rValue = m_d3dCbvGPUDescriptorStartHandle;
	rValue.ptr += ::gnCbvSrvDescriptorIncrementSize * m_nCbvIndex;
	return rValue;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorManager::GetSrvNextCPUHandle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE rValue = m_d3dSrvCPUDescriptorStartHandle;
	rValue.ptr += ::gnCbvSrvDescriptorIncrementSize * m_nSrvIndex;
	return rValue;
}

D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorManager::GetSrvNextGPUHandle() const
{
	D3D12_GPU_DESCRIPTOR_HANDLE rValue = m_d3dSrvGPUDescriptorStartHandle;
	rValue.ptr += ::gnCbvSrvDescriptorIncrementSize * m_nSrvIndex;
	return rValue;
}
