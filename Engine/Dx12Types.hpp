#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <dxgi1_6.h>

namespace dx12
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using DevicePtr = ComPtr<ID3D12Device>;
	using Factory6Ptr = ComPtr<IDXGIFactory6>;
	using SwapChain = ComPtr<IDXGISwapChain4>;
	using CmdListPtr = ComPtr<ID3D12CommandList>;
	using CmdQueuePtr = ComPtr<ID3D12CommandQueue>;
	using CmdAllocatorPtr = ComPtr<ID3D12CommandAllocator>;
	using ResourcePtr = ComPtr<ID3D12Resource>;
	using DescriptorHeapPtr = ComPtr<ID3D12DescriptorHeap>;
	using FencePtr = ComPtr<ID3D12Fence>;
	using DebugDevicePtr = ComPtr<ID3D12DebugDevice>;
	using AdapterPtr = ComPtr<IDXGIAdapter4>;
}