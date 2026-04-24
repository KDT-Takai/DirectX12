#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "../Singleton/Singleton.hpp"

class Direct3D : public Singleton<Direct3D> {
	DECLARE_SINGLETON(Direct3D)
private:
	Direct3D();
	~Direct3D();

	// デバッグレイヤーの有効化
	bool EnableDebugLayer();
	// 基本オブジェクトの生成
	bool D3D12CreateDeveice();
	// コマンドリストの作成
	bool CreateCommand();
	// スワップチェーンの生成
	bool CreateSwapChain(HWND hwnd);
	// ディスクリプタヒープの作成
	bool CreateDescriptorHeap();
	// レンダーターゲットビューの作成
	bool CreateRenderTargetView();
	// フェンスの作成
	bool CreateFence();

public:
	// 初期化
	bool Initialize(HWND hwnd);
	// 描画開始前の処理
	void Render();
private:
	// Direct3D デバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;

	// D3D12Device
	ID3D12Device* _dev = nullptr;
	// DXGIFactory
	IDXGIFactory6* _dxgiFactory = nullptr;
	// DXGISwapchain
	IDXGISwapChain4* _swapChain = nullptr;

	// コマンドリスト
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	
	// ディスクリプタヒープ
	ID3D12DescriptorHeap* _rtvHeap = nullptr;

	// バックバッファ
	std::vector<ID3D12Resource*> _backBuffers;

	// フェンス
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;

};