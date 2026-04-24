#pragma once
#include <windows.h>

#include "../Singleton/Singleton.hpp"

class Window : public Singleton<Window> {
	DECLARE_SINGLETON(Window)
private:
	Window();
	~Window();
public:
	// 初期化
	bool Initialize(const wchar_t* a_title, LONG a_width, LONG a_height);
	// メッセージ処理
	bool ProcessMessage();
	// ゲッター
	HWND GetHwnd() const { return m_hwnd; }
	HINSTANCE GetHInstance() const { return m_wndClass.hInstance; }
private:
	// ウィンドウクラスの登録
	bool RegisterWindowClass();
	// ウィンドウの生成
	bool CreateNativeWindow(const wchar_t* a_title, LONG a_width, LONG a_height);
private:
	HWND m_hwnd = nullptr;		// ウィンドウハンドル
	WNDCLASSEX m_wndClass = {};	// ウィンドウクラス
};