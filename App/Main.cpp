#include <windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#endif
// Window Class
#include "../Engine/Window/Window.hpp"
// Direct3D
#include "../Engine/Direct3D/Direct3D.hpp"

void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // _DEBUG
}

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	//DebugOutputFormatString("Show winodw test.");
	//std::getchar();
	
	// spdlogÇÃèâä˙âª
	spdlog::set_pattern("[%H:%M:%S][%^%l%$][%s:%#] %v");
	spdlog::set_level(spdlog::level::trace);
	
	Window::Create();
	bool windowFlag = Window::Get().Initialize(L"DirectX12Window", 1280, 720);
	if (windowFlag == false)
	{
		return -1;
	}
	Direct3D::Create();

	Direct3D::Get().Initialize(Window::Get().GetHwnd());
	while (true)
	{
		if (Window::Get().ProcessMessage())
		{
			break;
		}
		Direct3D::Get().Render();
	}
	Window::Delete();
	Direct3D::Delete();
	return 0;
}