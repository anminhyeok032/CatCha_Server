//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "Camera.h"
#include "Scene.h"
#include "Player.h"
#include "AudioManager.h"


CGameFramework::CGameFramework()
{
	dxgi_factory_ = NULL;
	dxgi_swap_chain_ = NULL;
	d3d12_device_ = NULL;

	for (int i = 0; i < swap_chain_buffers_; i++) d3d12_swap_chain_back_buffers_[i] = NULL;
	swap_chain_buffer_index_ = 0;

	d3d12_command_allocator_ = NULL;
	d3d12_command_queue_ = NULL;
	d3d12_command_list_ = NULL;

	d3d12_render_target_descriptor_heap_ = NULL;
	d3d12_depth_stencil_descriptor_heap_ = NULL;

	render_target_view_descriptor_increment_size_ = 0;
	depth_stencil_view_descriptor_increment_size_ = 0;

	fence_event_handle_ = NULL;
	d3d12_fence_ = NULL;
	for (int i = 0; i < swap_chain_buffers_; i++) d3d12_fence_valeues_[i] = 0;

	client_width_ = FRAME_BUFFER_WIDTH;
	client_height_ = FRAME_BUFFER_HEIGHT;

	scene_ = NULL;
	player_ = NULL;

	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	hinstance_ = hInstance;
	hwnd_ = hMainWnd;

	audio_manager_ = new CAudioManager();

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();

	BuildObjects();

	::SetCapture(hwnd_);
	::GetCursorPos(&old_cursor_position_);

	return(true);
}

//#define _WITH_SWAPCHAIN

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(hwnd_, &rcClient);
	client_width_ = rcClient.right - rcClient.left;
	client_height_ = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = swap_chain_buffers_;
	dxgiSwapChainDesc.BufferDesc.Width = client_width_;
	dxgiSwapChainDesc.BufferDesc.Height = client_height_;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = hwnd_;
	dxgiSwapChainDesc.SampleDesc.Count = (msaa_4x_enable_) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (msaa_4x_enable_) ? (msaa_4x_quality_levels_ - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	HRESULT hResult = dxgi_factory_->CreateSwapChain(d3d12_command_queue_, &dxgiSwapChainDesc, (IDXGISwapChain **)&dxgi_swap_chain_);
#endif

	if (!dxgi_swap_chain_)
	{
		MessageBox(NULL, L"Swap Chain Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	hResult = dxgi_factory_->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);
	swap_chain_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&dxgi_factory_);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != dxgi_factory_->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&d3d12_device_))) break;
	}

	if (!d3d12_device_)
	{
		hResult = dxgi_factory_->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void **)&d3d12_device_);
	}

	if (!d3d12_device_)
	{
		MessageBox(NULL, L"Direct3D 12 Device Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = d3d12_device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	msaa_4x_quality_levels_ = d3dMsaaQualityLevels.NumQualityLevels;
	msaa_4x_enable_ = (msaa_4x_quality_levels_ > 1) ? true : false;

	hResult = d3d12_device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&d3d12_fence_);
	for (UINT i = 0; i < swap_chain_buffers_; i++) d3d12_fence_valeues_[i] = 1;
	fence_event_handle_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (pd3dAdapter) pd3dAdapter->Release();

	::gnCbvSrvDescriptorIncrementSize = d3d12_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = d3d12_device_->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&d3d12_command_queue_);

	hResult = d3d12_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&d3d12_command_allocator_);

	hResult = d3d12_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d12_command_allocator_, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&d3d12_command_list_);
	hResult = d3d12_command_list_->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = swap_chain_buffers_;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = d3d12_device_->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&d3d12_render_target_descriptor_heap_);
	render_target_view_descriptor_increment_size_ = d3d12_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = d3d12_device_->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&d3d12_depth_stencil_descriptor_heap_);
	depth_stencil_view_descriptor_increment_size_ = d3d12_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = d3d12_render_target_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < swap_chain_buffers_; i++)
	{
		dxgi_swap_chain_->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&d3d12_swap_chain_back_buffers_[i]);
		d3d12_device_->CreateRenderTargetView(d3d12_swap_chain_back_buffers_[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += render_target_view_descriptor_increment_size_;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = client_width_;
	d3dResourceDesc.Height = client_height_;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (msaa_4x_enable_) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (msaa_4x_enable_) ? (msaa_4x_quality_levels_ - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = d3d12_depth_stencil_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
	d3d12_device_->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&d3d12_depth_stencil_buffer_);
/*
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_pd3dDepthStencilBuffer;
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
*/

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	d3d12_device_->CreateDepthStencilView(d3d12_depth_stencil_buffer_, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::OnResizeBackBuffers()
{
	WaitForGpuComplete();
	d3d12_command_list_->Reset(d3d12_command_allocator_, NULL);

	for (int i = 0; i < swap_chain_buffers_; i++) if (d3d12_swap_chain_back_buffers_[i]) d3d12_swap_chain_back_buffers_[i]->Release();
	if (d3d12_depth_stencil_buffer_) d3d12_depth_stencil_buffer_->Release();

#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

#else
	//m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	dxgi_swap_chain_->GetDesc(&dxgiSwapChainDesc);
	dxgi_swap_chain_->ResizeBuffers(swap_chain_buffers_, client_width_, client_height_, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
#endif
	swap_chain_buffer_index_ = 0;

	CreateRenderTargetViews();
	CreateDepthStencilView();

	d3d12_command_list_->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { d3d12_command_list_ };
	d3d12_command_queue_->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
}

void CGameFramework::ChangeSwapChainState()
{
	BOOL bFullScreenState = FALSE;
	dxgi_swap_chain_->GetFullscreenState(&bFullScreenState, NULL);
	dxgi_swap_chain_->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = client_width_;
	dxgiTargetParameters.Height = client_height_;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgi_swap_chain_->ResizeTarget(&dxgiTargetParameters);

	OnResizeBackBuffers();

}

#define WHEEL_DEGREE 120.f

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (scene_) scene_->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_RBUTTONDOWN:
			right_click_ = true;
			break;
		case WM_LBUTTONDOWN:
			// 05.10 수정: 기획에 맞춰서 마우스 입력 수정
			if (GetCapture() != hwnd_)
			{
				::SetCapture(hwnd_);
				::GetCursorPos(&old_cursor_position_);
			}
			left_click_ = true;
			break;
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			//::ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			break;
		case WM_MOUSEWHEEL:
			if (camera_->GetMode() == CameraMode::THIRD_PERSON)
			{
				CThirdPersonCamera* pCamera = ((CThirdPersonCamera*)camera_);
				pCamera->AddOffsetDistance(-(short)HIWORD(wParam) / WHEEL_DEGREE * pCamera->GetZoomScale());
				pCamera->SetOffset(pCamera->GetOffsetDistance(), pCamera->GetOffsetPitch());
			}
			else if (camera_->GetMode() == CameraMode::GHOST)
			{
				CGhostCamera* pCamera = ((CGhostCamera*)camera_);
				pCamera->SetMovingSpeed(pCamera->GetMovingSpeed() + (short)HIWORD(wParam) / WHEEL_DEGREE * CAMERA_ACCELERATION);
			}
			break;
		default:
			break;
	}
}

#define VK_W             0x57
#define VK_A             0x41
#define VK_S             0x53
#define VK_D             0x44
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (scene_) scene_->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_KEYDOWN:
			if (wParam == VK_CONTROL) control_key_ = true;

			switch (wParam)
			{
			case VK_W:
			case VK_A:
			case VK_S:
			case VK_D:
			//case VK_SPACE:
				// [CS] key input이 있을시 패킷 전송
				press_keyboard_movement_ = true;
				input_key_ = (static_cast<uint8_t>(wParam) << 1) | true;
				break;
			case 'r':
			case 'R':
				player_->set_is_ether(!player_->is_ether());
				break;
			case VK_CONTROL:
				control_key_ = true;
				break;
			default:
				break;
			}
		break;
		case WM_KEYUP:		

			switch (wParam)
			{
			case VK_W:
			case VK_A:
			case VK_S:
			case VK_D:
				// [CS] key input이 있을시 패킷 전송
				if (true == press_keyboard_movement_)
				{
					input_key_ = (static_cast<uint8_t>(wParam) << 1) | false;
					player_->SendInput(input_key_);
					break;
				}
			case VK_CONTROL:
				control_key_ = false;
			break;
			case VK_SPACE:
				//player_->InputActionRoll(0);
			break;
			case VK_ESCAPE:
				::PostQuitMessage(0);
				exit(0);
				break;
			case VK_OEM_MINUS:
			{
				CCamera* pCamera = new CGhostCamera(camera_);
				delete camera_;
				camera_ = pCamera;
				camera_->CreateShaderVariables(d3d12_device_, d3d12_command_list_);
				player_->SetCamera(camera_);
			}
				break;
			case VK_RETURN:
				break;
			case VK_F9:
			{
				ChangeSwapChainState();
				break;
			}
			case VK_F10:
				break;
			default:
				break;
			}
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				game_timer_.Stop();
			else
				game_timer_.Start();
			break;
		}
		case WM_SIZE:
		{
			client_width_ = LOWORD(lParam);
			client_height_ = HIWORD(lParam);

			OnResizeBackBuffers();
			break;
		}
		
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(fence_event_handle_);

	if (d3d12_depth_stencil_buffer_) d3d12_depth_stencil_buffer_->Release();
	if (d3d12_depth_stencil_descriptor_heap_) d3d12_depth_stencil_descriptor_heap_->Release();

	for (int i = 0; i < swap_chain_buffers_; i++) if (d3d12_swap_chain_back_buffers_[i]) d3d12_swap_chain_back_buffers_[i]->Release();
	if (d3d12_render_target_descriptor_heap_) d3d12_render_target_descriptor_heap_->Release();

	if (d3d12_command_allocator_) d3d12_command_allocator_->Release();
	if (d3d12_command_queue_) d3d12_command_queue_->Release();
	if (d3d12_command_list_) d3d12_command_list_->Release();

	if (d3d12_fence_) d3d12_fence_->Release();

	dxgi_swap_chain_->SetFullscreenState(FALSE, NULL);
	if (dxgi_swap_chain_) dxgi_swap_chain_->Release();
    if (d3d12_device_) d3d12_device_->Release();
	if (dxgi_factory_) dxgi_factory_->Release();
}

void CGameFramework::BuildObjects()
{
	d3d12_command_list_->Reset(d3d12_command_allocator_, NULL);

	camera_ = new CThirdPersonCamera;
	player_ = new CPlayer(d3d12_device_, d3d12_command_list_, camera_);
	camera_->SetPlayer(player_);
	((CThirdPersonCamera*)camera_)->ResetFromPlayer();

	scene_ = new CScene;
	if (scene_) scene_->BuildObjects(d3d12_device_, d3d12_command_list_, player_, audio_manager_);


	d3d12_command_list_->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { d3d12_command_list_ };
	d3d12_command_queue_->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	//audio_manager_->AddTrack("BackGroundMusic");
	//audio_manager_->PlayTrack("BackGroundMusic");
	
	if (scene_) scene_->ReleaseUploadBuffers();
	//if (player_) player_->ReleaseUploadBuffers();

	game_timer_.Reset();
}

void CGameFramework::ReleaseObjects()
{
	//if (player_) delete player_;

	if (scene_) scene_->ReleaseObjects();
	if (scene_) delete scene_;
	if (camera_) delete camera_;
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	float elapsed_time = game_timer_.GetTimeElapsed();
	if (GetKeyboardState(pKeysBuffer) && scene_) bProcessedByScene = scene_->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;
		if (pKeysBuffer['W'] & 0xF0) {
			dwDirection |= DIR_FORWARD;
		}
		if (pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeysBuffer['E'] & 0xF0) dwDirection |= DIR_UP;
		if (pKeysBuffer['Q'] & 0xF0) dwDirection |= DIR_DOWN;
		if (pKeysBuffer[' '] & 0xF0) player_->InputActionRoll(dwDirection);

		XMFLOAT2 delta_xy = XMFLOAT2(0.f, 0.f);
		POINT ptCursorPos;
		if (GetCapture() == hwnd_)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			delta_xy.x = (float)(ptCursorPos.x - old_cursor_position_.x) * 0.3f;
			//delta_xy.y = (float)(ptCursorPos.y - old_cursor_position_.y) * 0.3f;
			SetCursorPos(old_cursor_position_.x, old_cursor_position_.y);
		}

		player_->InputActionRotate(delta_xy, elapsed_time);
		player_->InputActionMove(dwDirection, elapsed_time);

		if (0 == dwDirection)
		{
			
			press_keyboard_movement_ = false;	// [CS] 키보드 입력 없을시 보내지 못하게
		}

		// [CS] Move Packet Send
		if(press_keyboard_movement_)
		{	
			player_->SendInput(input_key_);
		}

		//if (press_keyboard_movement_ && player_->animation_state() == CPlayer::PlayerAnimation::Roll)

		if(camera_->GetMode() == CameraMode::GHOST) 
			((CGhostCamera*)camera_)->Move(dwDirection, elapsed_time);

	}

	if (left_click_)
	{
		click_time_ += elapsed_time;
		if (click_time_ > both_click_delay_time_)
		{
			player_->InputActionAttack(PlayerAttackType::LeftAttack);
			left_click_ = false;
			click_time_ = 0.f;
		}
		if (control_key_)
		{
			player_->InputActionAttack(PlayerAttackType::ControlAttack);
		}
	}
	if (right_click_)
	{
		click_time_ += elapsed_time;
		if (click_time_ > both_click_delay_time_)
		{
			player_->InputActionAttack(PlayerAttackType::RightAttack);
			right_click_ = false;
			click_time_ = 0.f;
		}
	}
	if (left_click_ && right_click_)
	{
		player_->InputActionAttack(PlayerAttackType::BothAttack);
		click_time_ = 0.f;
		left_click_ = right_click_ = false;
	}

	//scene_->CollisionCheck();
	
	// [SC] 다른 플레이어 상태 업데이트
	scene_->UpdatePlayerlist(elapsed_time);

	player_->Update(elapsed_time);
	
	camera_->Update(elapsed_time);

}

void CGameFramework::AnimateObjects()
{
	if (scene_) scene_->AnimateObjects(game_timer_.GetTimeElapsed());
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++d3d12_fence_valeues_[swap_chain_buffer_index_];
	HRESULT hResult = d3d12_command_queue_->Signal(d3d12_fence_, nFenceValue);

	if (d3d12_fence_->GetCompletedValue() < nFenceValue)
	{
		hResult = d3d12_fence_->SetEventOnCompletion(nFenceValue, fence_event_handle_);
		::WaitForSingleObject(fence_event_handle_, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	swap_chain_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();
	//m_nSwapChainBufferIndex = (m_nSwapChainBufferIndex + 1) % m_nSwapChainBuffers;

	UINT64 nFenceValue = ++d3d12_fence_valeues_[swap_chain_buffer_index_];
	HRESULT hResult = d3d12_command_queue_->Signal(d3d12_fence_, nFenceValue);

	if (d3d12_fence_->GetCompletedValue() < nFenceValue)
	{
		hResult = d3d12_fence_->SetEventOnCompletion(nFenceValue, fence_event_handle_);
		::WaitForSingleObject(fence_event_handle_, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{    
	game_timer_.Tick(0.0f);
	
	ProcessInput();

	HRESULT hResult = d3d12_command_allocator_->Reset();
	hResult = d3d12_command_list_->Reset(d3d12_command_allocator_, NULL);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = d3d12_swap_chain_back_buffers_[swap_chain_buffer_index_];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	d3d12_command_list_->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = d3d12_render_target_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (swap_chain_buffer_index_ * render_target_view_descriptor_increment_size_);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	d3d12_command_list_->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = d3d12_depth_stencil_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
	d3d12_command_list_->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	d3d12_command_list_->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	AnimateObjects();

	if (scene_) scene_->Render(d3d12_command_list_, camera_, game_timer_.GetTimeElapsed());

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif


	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	d3d12_command_list_->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = d3d12_command_list_->Close();
	
	ID3D12CommandList *ppd3dCommandLists[] = { d3d12_command_list_ };
	d3d12_command_queue_->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	dxgi_swap_chain_->Present(0, 0);
#endif
#endif

//	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	MoveToNextFrame();

	game_timer_.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(hwnd_, m_pszFrameRate);
}

