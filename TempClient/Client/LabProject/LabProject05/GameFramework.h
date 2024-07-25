#pragma once

#include "Timer.h"

class CScene;
class CPlayer;
class CCamera;
class CAudioManager;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void OnResizeBackBuffers();
	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE					hinstance_;
	HWND						hwnd_; 

	int							client_width_;
	int							client_height_;
        
	IDXGIFactory4				*dxgi_factory_ = NULL;
	IDXGISwapChain3				*dxgi_swap_chain_ = NULL;
	ID3D12Device				*d3d12_device_ = NULL;

	bool						msaa_4x_enable_ = false;
	UINT						msaa_4x_quality_levels_ = 0;

	static const UINT			swap_chain_buffers_ = 2;
	UINT						swap_chain_buffer_index_;

	std::array<ID3D12Resource*, swap_chain_buffers_> d3d12_swap_chain_back_buffers_;
	ID3D12DescriptorHeap		*d3d12_render_target_descriptor_heap_ = NULL;
	UINT						render_target_view_descriptor_increment_size_;

	ID3D12Resource				*d3d12_depth_stencil_buffer_ = NULL;
	ID3D12DescriptorHeap		*d3d12_depth_stencil_descriptor_heap_ = NULL;
	UINT						depth_stencil_view_descriptor_increment_size_;

	ID3D12CommandAllocator		*d3d12_command_allocator_ = NULL;
	ID3D12CommandQueue			*d3d12_command_queue_ = NULL;
	ID3D12GraphicsCommandList	*d3d12_command_list_ = NULL;

	ID3D12Fence					*d3d12_fence_ = NULL;
	std::array<UINT64, swap_chain_buffers_>	d3d12_fence_valeues_;
	HANDLE						fence_event_handle_;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					game_timer_;

	CScene						*scene_ = NULL;
	CPlayer						*player_ = NULL;
	CCamera						*camera_ = NULL;

	CAudioManager* audio_manager_ = NULL;

	POINT						old_cursor_position_;

	//마우스 양클릭을 체크하기 위한 대기시간
	const float both_click_delay_time_ = 0.1;
	float click_time_ = 0.f;

	//마우스 각 클릭에 대한 트리거 버튼을 누르면 true가 되고 입력처리되면 false가 됨
	bool left_click_ = false;
	bool right_click_ = false;

	//컨트롤 키 값
	bool control_key_ = false;

	// [CS] 현재 키보드가 입력되고 있는지
	bool press_keyboard_movement_ = false;
	uint8_t input_key_;

	_TCHAR						m_pszFrameRate[50];
};

