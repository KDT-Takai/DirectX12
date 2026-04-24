#include "Direct3D.hpp"
#include "../Log.hpp"

Direct3D::Direct3D()
{
}

Direct3D::~Direct3D()
{
}

bool Direct3D::EnableDebugLayer()
{
#ifdef _DEBUG
    ID3D12Debug* debugController = nullptr;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        // デバッグレイヤーの有効化
        debugController->EnableDebugLayer();
		debugController->Release(); // 使い終わったら解放
        LOG_INFO("デバッグレイヤーを有効化しました");
    }
    else
    {
        LOG_CRITICAL("デバッグレイヤーの有効化に失敗しました");
        return false;
    }
#endif

    return true;
}

bool Direct3D::D3D12CreateDeveice()
{
	LOG_INFO("Direct3Dデバイスの生成開始");
    // 教科書ではauto型でしている
    HRESULT result = S_OK;
    // DXGIFactoryの作成
#ifdef _DEBUG
    result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
    result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
    // 1：DXGIFacotryの生成
    // アダプターを列挙するためにDXGIFactoryオブジェクトを生成
    // オブジェクト変数をIDXGIFactory6*型としているため、
    // CreateDXGIFactory1()関数を使って生成する
	// これでDXGIFactoryオブジェクトが_dxgiFactoryに入る
    //result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
    if (FAILED(result)) return false;

    // 2：アダプターの選定
    // 利用可能なアダプターを列挙し、列挙されたアダプターをstd::vectorに入れておく
    std::vector<IDXGIAdapter*> adapters;
    // 特定の名前を持つアダプターオブジェクトが入る
    IDXGIAdapter* tmpAdapter = nullptr;
    for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        adapters.push_back(tmpAdapter);
    }
    IDXGIAdapter* useAdapter = nullptr;

    // アダプターを識別するための情報（DXGI_ADAPTER_DESC構造体）をループで取得
    // DXGI_ADAPTER_DESC構造体にはDescriptionメンバ変数がある
    // そこにアダプターの名前が格納される
    // それを利用し、特定のアダプターを探し出す
    for (auto adpt : adapters)
    {
        DXGI_ADAPTER_DESC adesc = {};
        // アダプターの説明オブジェクト取得
        adpt->GetDesc(&adesc);
        std::wstring strDesc = adesc.Description;
        // 探したいアダプターの名前を確認
        if (strDesc.find(L"NVIDIA") != std::wstring::npos)
        {
            // 今回はuseAdapterを最終の出力とするため、見つかったらuseAdapterに入れる
            // tmpAdapter = adpt;
			useAdapter = adpt;
            break;
        }
    }
    // NVIDIAが見つからなかったらリストの０番目を使う保険をやっておく
    if (useAdapter == nullptr && !adapters.empty())
    {
        useAdapter = adapters[0];
    }

    // 3：デバイスの生成
    // 一部のグラフィックボードでは指定したフィーチャーレベルでは
    // 呼び出しに失敗する可能性がある。
    // その場合はフィーチャーレベルを１つずつ落としながら
    // 合致する設定を探す必要がある
    D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    // 初期化は失敗にする
    result = E_FAIL;
    for (auto lv : levels)
    {
        if (D3D12CreateDevice(useAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
        {
            featureLevel = lv;  // 保存
            result = S_OK;      // 成功
            break; // 生成可能なバージョンが見つかったらループを打ち切り
        }
    }

    // _dxgiFactoryがnullptrでないか確認
    if (_dxgiFactory == nullptr)
    {
        LOG_CRITICAL("DXGIFactoryの生成に失敗");
        return false;
    }
	// _devがnullptrでないか確認
    if (_dev == nullptr)
    {
        // ログを出して原因を知らせる
        // （Window.cppで作ったマクロが使えるはずです）
        LOG_CRITICAL("Direct3Dデバイスの生成に失敗。対応GPUがない可能性");
        return false;
    }

    return true; // 成功
}

bool Direct3D::CreateCommand()
{
	LOG_INFO("コマンド関連オブジェクトの生成開始");

    HRESULT result = S_OK;

    // コマンドアロケータの生成
    result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
    if (FAILED(result))
    {
        LOG_CRITICAL("コマンドアロケータの生成に失敗");
        return false;
    }

    // コマンドリストの生成
    result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
    if (FAILED(result))
    {
        LOG_CRITICAL("コマンドリストの生成に失敗");
        return false;
    }

	// メモ 作った直後のコマンドリストは記録状態になっているのでいったん閉じる
    _cmdList->Close();

    // コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    // タイムアウト無し
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    // アダプターを一つしか使わないときは０
	cmdQueueDesc.NodeMask = 0;
    // プライオリティは特に指定なし
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    // コマンドリストと合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // キュー作成
    result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
    if (FAILED(result))
    {
        LOG_CRITICAL("コマンドキューの生成に失敗");
        return false;
	}

    return true; // 成功
}

bool Direct3D::CreateSwapChain(HWND hwnd)
{
	LOG_INFO("スワップチェーンの生成開始");

    HRESULT result = S_OK;

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

    RECT clientRect = {};
    GetClientRect(hwnd, &clientRect);

    // 2. 取得したサイズをセットする
    swapchainDesc.Width = clientRect.right - clientRect.left;
    swapchainDesc.Height = clientRect.bottom - clientRect.top;

    //swapchainDesc.Width = widnow_width;
    //swapchainDesc.Height = widnow_height;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
    // バックバッファーは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	// フリップ後は速やかに破棄
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    // 特に指定なし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // ウィンドウ⇔フルスクリーンの切り替え可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    // 本来はQueryInterfaceなどを用いて
    // IDXGISwapChain4*への変換チェックを行うが、今回は分かりやすさ有線でキャストで対応
    result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&_swapChain);

    if (FAILED(result))
    {
        LOG_CRITICAL("スワップチェーンの生成に失敗");
		return false;  // 失敗
	}
    
    return true; // 成功
}

bool Direct3D::CreateDescriptorHeap()
{
	LOG_INFO("ディスクリプタヒープの生成開始");

    HRESULT result = S_OK;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビューなのでRTV
	heapDesc.NodeMask = 0;
    heapDesc.NumDescriptors = 2;    // 表裏の２つ
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;   // NONE

    ID3D12DescriptorHeap* rtvHeaps = nullptr;
    result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeap));

    if (FAILED(result))
    {
        LOG_CRITICAL("ディスクリプタヒープの生成に失敗");
        return false; // 失敗
    }

    return true;
}

bool Direct3D::CreateRenderTargetView()
{
	LOG_INFO("レンダーターゲットビューの生成開始");

    HRESULT result = S_OK;

	DXGI_SWAP_CHAIN_DESC swcDesc = {};

	result = _swapChain->GetDesc(&swcDesc);

	if (FAILED(result))
    {
		LOG_CRITICAL("スワップチェーンの情報の取得に失敗");
		return false;
    }

	_backBuffers.resize(swcDesc.BufferCount);

    for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
    {
		// バックバッファーの取得
		result = _swapChain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
        if (FAILED(result))
        {
            LOG_CRITICAL("バックバッファーの取得に失敗");
            return false;
        }
        // あくまで先頭のアドレス　１番目以降のディスクリプタを取得するには１つ分後ろにずらす必要がある
        // ビューの種類によってディスクリプタが必要とするサイズが違う
        // 受け渡しに使用するのはハンドルであってアドレスそのものではない
        //_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, _rtvHeap->GetCPUDescriptorHandleForHeapStart());

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        _dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);

        // 別の書き方
   //     D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
   //     for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
   //     {
   //         // レンダーターゲットを生成する
   //         _dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
   //         // ポインターをずらす
			//handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
   //     }
    }

    return true;
}

bool Direct3D::CreateFence()
{
	LOG_INFO("フェンスの生成開始");

    HRESULT result = S_OK;

    result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    if(FAILED(result))
    {
        LOG_CRITICAL("フェンスの生成に失敗");
        return false;
	}

    return true;
}

bool Direct3D::Initialize(HWND hwnd)
{
	// デバッグレイヤーの有効化
    if (!EnableDebugLayer())
    {
        return false;
    }
    // デバイス生成
    if (!D3D12CreateDeveice())
    {
        return false;
    }
    // コマンド系生成
    if (!CreateCommand())
    {
        return false;
    }
    // スワップチェーン生成
    if (!CreateSwapChain(hwnd))
    {
        return false;
    }
	// ディスクリプタヒープ生成
    if (!CreateDescriptorHeap())
    {
        return false;
	}
	// レンダーターゲットビュー生成
    if (!CreateRenderTargetView())
    {
        return false;
	}
	// フェンス生成
    if (!CreateFence())
    {
        return false;
    }
    return true;
}

void Direct3D::Render()
{
    if (_cmdList == nullptr || _cmdAllocator == nullptr || _swapChain == nullptr)
    {
        LOG_CRITICAL("コマンドリスト、コマンドアロケータ、スワップチェーンのいずれかがnullptr");
        return;
	}

    // フレームの開始時にアロケータとコマンドリストのリセット
    _cmdAllocator->Reset();
    _cmdList->Reset(_cmdAllocator, nullptr);

	// バックバッファーのインデックスを取得
    auto bbIdx = _swapChain->GetCurrentBackBufferIndex();

	// リソースバリアの設定
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;  // 推移
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;       // 特に指定なし
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx];     // バックバッファーリソース
    BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;   // 教科書では０やけど全てのサブリソースを対象にする
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;  // 直前はPRESENT状態
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // 今からレンダーターゲット状態
	_cmdList->ResourceBarrier(1, &BarrierDesc); // バリア指定実行

    // コマンドリストのリセット
	auto rtvH = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _cmdList->OMSetRenderTargets(1,&rtvH, true, nullptr);


    // 画面クリア
	float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    _cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

    // レンダーターゲットからプレゼント状態に遷移するためのバリア
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // 直前はレンダーターゲット状態
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;  // 今からPRESENT状態
	_cmdList->ResourceBarrier(1, &BarrierDesc); // バリア指定実行

    // 命令のクローズ
    _cmdList->Close();

    // コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _cmdList };
    _cmdQueue->ExecuteCommandLists(1, cmdlists);

    // フリップ
    _swapChain->Present(1, 0);

    _cmdQueue->Signal(_fence, ++_fenceVal);
    if (_fence->GetCompletedValue() != _fenceVal)
    {
        // イベントハンドルの取得
        auto event = CreateEvent(nullptr, false, false, nullptr);
        _fence->SetEventOnCompletion(_fenceVal, event);
        // イベントが発生するまで待ち続ける(INFINTE)
        WaitForSingleObject(event, INFINITE);
        // イベントハンドルを閉じる
        CloseHandle(event);
    }
}