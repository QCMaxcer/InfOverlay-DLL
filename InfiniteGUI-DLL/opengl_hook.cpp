#include "opengl_hook.h"
#include <Windows.h>
#include "detours\titan_hook.h"
#include <iostream>
#include <stdexcept>

#include "menu.h"
#include "ItemManager.h"
#include "GameStateDetector.h"
#include "gui.h"

#include "Images.h"
#include "pics\MCInjector-small.h"
#include <thread>

#include "GameWindowTool.h"
#include "Init.hpp"
#include "Motionblur.h"
#include "MusicInfoItem.h"

//#include <base/voyage.h>
//#include <mutex>
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool detour_wgl_swap_buffers(HDC hdc);
static LRESULT CALLBACK wndproc_hook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void opengl_hook::lwjgl2FullscreenHandler() {
	remove_hook();
	clean();
	init();
	handle_window = WindowFromDC(handle_device_ctx);
	o_wndproc = (WNDPROC)SetWindowLongPtrW(handle_window, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
	custom_gl_ctx = wglCreateContext(handle_device_ctx);
	gui.logoTexture.id = NULL;
	RECT area;
	GetClientRect(handle_window, &area);
	screen_size.x = area.right - area.left;
	screen_size.y = area.bottom - area.top;
}

static void WndprocDestory()
{
	opengl_hook::gui.done = true;
	auto start = std::chrono::steady_clock::now();
	while (!opengl_hook::g_isDetaching.load())
	{
		std::this_thread::yield();

		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 1000)
		{
			Uninit();
			break; // 超时 1s
		}
	}
}

static LRESULT CALLBACK wndproc_hook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool isRepeat = (lParam & (1 << 30)) != 0;
	switch (message)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		if(!KeyState::GetKeyDown(GameWindowTool::Instance().GetFullscreenVK()) && !GameStateDetector::Instance().IsFullScreenClicked())
			WndprocDestory();
		break;
	case WM_KEYDOWN:
	{
		if (wParam == Menu::Instance().GetKeyBind() && !isRepeat)
		{
			Menu::Instance().isEnabled = !Menu::Instance().isEnabled;
			Menu::Instance().Toggle();
		}
		break;
	}
	case WM_INPUT:
	{
		UINT size;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		BYTE* buffer = new BYTE[size];
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)buffer;
		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			int dx = raw->data.mouse.lLastX;
			int dy = raw->data.mouse.lLastY;

			// dx / dy 就是你的视角移动量
			GameStateDetector::Instance().ProcessMouseMovement(dx, dy);
		}
		delete[] buffer;
		break;
	}
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		opengl_hook::screen_size = { width, height };
		if (Menu::Instance().isEnabled)
		{
			Menu::Instance().Repos();
			//取消cursor范围限制
			ClipCursor(NULL);
		}
		break;
	}
	case WM_MOVE:
		if (Menu::Instance().isEnabled)
		{
			//取消cursor范围限制
			ClipCursor(NULL);
		}
	default:
	{
		break;
	}
	}
	bool state;
	if (message == WM_KEYDOWN || message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
		message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN)
		state = true;
	else if (message == WM_KEYUP || message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
		message == WM_MBUTTONUP || message == WM_XBUTTONUP)
		state = false;
	else
		state = true;

	ItemManager::Instance().ProcessKeyEvents(state, isRepeat, wParam);
	if (!GameStateDetector::Instance().IsInGame())
	{
		// 只有 UI 激活时才把消息交给 ImGui 处理
		ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
		if(Menu::Instance().isEnabled)
		{
			switch (message)
			{
			case WM_SETCURSOR:
				//if (LOWORD(lParam) == HTCLIENT)
				//{
				//	SetCursor(LoadCursor(nullptr, IDC_ARROW));
				//}
			case WM_SYSDEADCHAR:
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_CHAR:
			case WM_DEADCHAR:
			case WM_MOUSEMOVE:
			case WM_MOUSEWHEEL:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_XBUTTONDBLCLK:
			case WM_MOUSEHOVER:
			case WM_INPUT:
				return TRUE;
			default:
				break;
			}
		}

	}
	// 始终把消息交回原来的窗口过程（不管 UI 是否激活）
	return CallWindowProcW(opengl_hook::o_wndproc, hWnd, message, wParam, lParam);
}



TitanHook<decltype(&detour_wgl_swap_buffers)>wgl_swap_buffers_hook;
void opengl_hook::init()
{
	HMODULE h_ogl_32 = GetModuleHandleW(L"opengl32.dll");
	if (!h_ogl_32)
	{
		throw std::runtime_error("unable to get ogl32 handle!");
	}
	LPVOID pfunc_wgl_swap_buffers = (LPVOID)GetProcAddress(h_ogl_32, "wglSwapBuffers");
	if (!pfunc_wgl_swap_buffers)
	{
		throw std::runtime_error("unable to get wglswapbuffer's function address!");
	}
	wgl_swap_buffers_hook.InitHook(pfunc_wgl_swap_buffers, detour_wgl_swap_buffers);
	wgl_swap_buffers_hook.SetHook();

	return;
}

void opengl_hook::remove_hook()
{
	SetWindowLongPtrW(handle_window, GWLP_WNDPROC, (LONG_PTR)o_wndproc);
	wgl_swap_buffers_hook.RemoveHook();
}

bool opengl_hook::clean()
{
	Menu::Instance().DestoryMenuBlur();
	Motionblur::Instance().Destroy();
	wglMakeCurrent(handle_device_ctx, opengl_hook::o_gl_ctx);
	wglDeleteContext(custom_gl_ctx);
	gui.clean();
	//ImGui::DestroyContext();
	return false;
}

bool detour_wgl_swap_buffers(HDC hdc)
{
	if (opengl_hook::gui.done)
	{
		return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
	} //detach
	//glPushMatrix();
	opengl_hook::rendering = true;
	opengl_hook::o_gl_ctx = wglGetCurrentContext();
	opengl_hook::handle_device_ctx = hdc;
	static std::once_flag flag;
	std::call_once(flag, [&]
		{
			// 只执行一次
			opengl_hook::handle_window = WindowFromDC(opengl_hook::handle_device_ctx);
			opengl_hook::custom_gl_ctx = wglCreateContext(opengl_hook::handle_device_ctx);

			RECT area;
			GetClientRect(opengl_hook::handle_window, &area);

			opengl_hook::screen_size.x = area.right - area.left;
			opengl_hook::screen_size.y = area.bottom - area.top;

			opengl_hook::o_wndproc = (WNDPROC)SetWindowLongPtrW(opengl_hook::handle_window, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
			RAWINPUTDEVICE rid;
			rid.usUsagePage = 0x01;
			rid.usUsage = 0x02; // Mouse
			rid.dwFlags = RIDEV_INPUTSINK;
			rid.hwndTarget = opengl_hook::handle_window;
			RegisterRawInputDevices(&rid, 1, sizeof(rid));
			Fonts::init();

		});
	if (WindowFromDC(hdc) != opengl_hook::handle_window) return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
	//渲染代码
	wglMakeCurrent(hdc, opengl_hook::custom_gl_ctx);
	if(wglGetCurrentContext())
	{
		if (!opengl_hook::gui.isInit)
		{
				opengl_hook::gui.init();
		}
		if (!opengl_hook::gui.logoTexture.id) opengl_hook::gui.logoTexture.id = LoadTextureFromMemory(logo, logoSize, &opengl_hook::gui.logoTexture.width, &opengl_hook::gui.logoTexture.height);
		opengl_hook::gui.render();
		if (opengl_hook::gui.done)
		{
			MusicInfoItem::Instance().ShutDown();
			if (opengl_hook::gui.logoTexture.id)
			{
				glDeleteTextures(1, &opengl_hook::gui.logoTexture.id);
				opengl_hook::gui.logoTexture.id = 0;
			}
		}
	}
	wglMakeCurrent(hdc, opengl_hook::o_gl_ctx);
	//glPopMatrix();
	opengl_hook::rendering = false;
	return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
}
