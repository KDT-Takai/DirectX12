#include "Window.hpp"
#include <tchar.h>

#include "../Log.hpp"

Window::Window()
{
	
}

Window::~Window()
{
	if (m_wndClass.lpszClassName)
	{
		// クラスは使わないので登録解除
		UnregisterClass(m_wndClass.lpszClassName, m_wndClass.hInstance);
	}
}

static LRESULT CALLBACK WindowProcedure(HWND a_hwnd, UINT a_msg, WPARAM a_wparam, LPARAM a_lparam)
{
	// ウィンドウが破棄されたら呼ばれる
	if (a_msg == WM_DESTROY)
	{
		// OSに対してこのアプリは終了と伝える
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(a_hwnd, a_msg, a_wparam, a_lparam);
}

bool Window::Initialize(const wchar_t* a_title, LONG a_width, LONG a_height)
{
	// ウィンドウクラスの登録
	bool RegisterWindowFlag = RegisterWindowClass();
	if (!RegisterWindowFlag)
	{
		LOG_CRITICAL("初期化中断：ウィンドウクラスの登録失敗");
		return false;
	}
	// ウィンドウクラスの生成
	bool CreateNativeWindowFlag = CreateNativeWindow(a_title, a_width, a_height);
	if (!CreateNativeWindowFlag)
	{
		LOG_CRITICAL("初期化中断：ウィンドウ生成失敗");
		return false;
	}
	// ウィンドウの表示
	ShowWindow(m_hwnd, SW_SHOW);
	LOG_INFO("ウィンドウ初期化に成功。ウィンドウの表示");
	return true;
}

bool Window::ProcessMessage()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			return true;
		}
	}
    return false;
}

bool Window::RegisterWindowClass()
{
	// ウィンドウクラスの登録
	m_wndClass.cbSize = sizeof(WNDCLASSEX);
	// コールバック関数の指定
	m_wndClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	// アプリケーションクラス名
	m_wndClass.lpszClassName = _T("DirectX12");
	// ハンドルの取得
	m_wndClass.hInstance = GetModuleHandle(nullptr);
	// アプリケーションクラス
	// ウィンドウクラスの指定をOSに伝える
	if (!RegisterClassEx(&m_wndClass))
	{
		DWORD errorCode = GetLastError();
		LOG_ERROR("ウィンドウクラス登録に失敗。エラーコード: {}", errorCode);
		return false;
	}
	LOG_INFO("ウィンドウクラス登録に成功");
	return true;
}

bool Window::CreateNativeWindow(const wchar_t* a_title, LONG a_width, LONG a_height)
{
	// ウィンドウサイズの指定
	RECT wrc = { 0, 0, a_width, a_height };
	// ウィンドウサイズの補正
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	// タイトルバーの文字
	LPCWSTR lpWindowName = a_title;
	// ウィンドウサイズ
	int nWidth = wrc.right - wrc.left;	// ウィンドウ幅
	int nHeight = wrc.bottom - wrc.top;	// ウィンドウ高
	// ウィンドウオブジェクトの生成
	m_hwnd = CreateWindow(
		m_wndClass.lpszClassName,		// クラス名指定
		lpWindowName,			// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	// タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,			// 表示x座標はOSにお任せ
		CW_USEDEFAULT,			// 表示y座標はOSにお任せ
		nWidth,					// ウィンドウ幅
		nHeight,				// ウィンドウ高
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		m_wndClass.hInstance,	// 呼び出しアプリケーションハンドル
		nullptr);				// 追加パラメーター

	// 生成失敗チェック
	if (!m_hwnd)
	{
		DWORD errorCode = GetLastError();
		LOG_ERROR("ウィンドウ生成に失敗。エラーコード: {}", errorCode);
		return false;
	}
	LOG_INFO("ウィンドウ生成に成功");
	return true;
}