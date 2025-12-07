#include "Window.h"

std::unordered_set<std::wstring> platform::Window::s_registeredClasses;


platform::Window::~Window()
{
	if (m_hWnd) {
		::DestroyWindow(m_hWnd);
	}
}


LRESULT platform::Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto it = m_messageHandlers.find(uMsg);
	if (it != m_messageHandlers.end())
	{
		it->second(wParam, lParam);
		return 0;
	}
	return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK platform::Window::WndProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* window = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

		if (window)
		{
			window->m_hWnd = hwnd;
		}
	}
	else
	{
		window = reinterpret_cast<Window*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (window)
	{
		return window->HandleMessage(uMsg, wParam, lParam);
	}
	else
	{
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


bool platform::Window::RegisterWindowClass(HINSTANCE hInstance, UINT style, const std::wstring& className)
{
	if (s_registeredClasses.count(className))
	{
		return true;
	}

	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX),
		style,
		WndProc,
		0L, 0L,
		hInstance,
		::LoadIconW(NULL, IDI_APPLICATION),
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		className.c_str(),
		NULL };

	if (::RegisterClassEx(&wc))
	{
		s_registeredClasses.insert(className);
		return true;
	}
	return false;
}
bool platform::Window::Create(
	HINSTANCE hInstance,
	const std::wstring& title,
	int width, int height, DWORD style,
	const std::wstring& className, HWND parent)
{
	if (m_hWnd)
	{
		return false;
	}

	m_hWnd = ::CreateWindowEx(
		WS_EX_CLIENTEDGE,
		className.c_str(),
		title.c_str(),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, height,
		parent, NULL, hInstance, this);

	if (m_hWnd)
	{
		return true;
	}
	return false;
}
bool platform::Window::ShowWindow(int cmdShow)
{
	return ::ShowWindow(m_hWnd, cmdShow);
}

bool platform::Window::SetWindowTitle(const std::wstring& title)
{
	return ::SetWindowTextW(m_hWnd, title.c_str());
}
std::wstring platform::Window::GetWindowTitle() const
{
	int length = ::GetWindowTextLengthW(m_hWnd);
	if (length == 0)
	{
		return L"";
	}

	std::wstring title(length, L'\0');
	int charsCopied = ::GetWindowTextW(m_hWnd, &title[0], length + 1);
	if (charsCopied > 0)
	{
		title.resize(charsCopied);
	}
	else
	{
		return L"";
	}
	return title;
}

bool platform::Window::SetWindowSize(int width, int height)
{
	return ::SetWindowPos(
		m_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}
platform::Size platform::Window::GetClientSize() const
{
	RECT clientRect;
	if (::GetClientRect(m_hWnd, &clientRect))
	{
		return { clientRect.right, clientRect.bottom };
	}
	return { -1, -1 };
}

void platform::Window::SetCursorStyle(platform::Window::CursorStyle style)
{
	HCURSOR cursor = nullptr;
	switch (style)
	{
	case platform::Window::CursorStyle::Arrow:
		cursor = ::LoadCursor(NULL, IDC_ARROW);
		break;
	case platform::Window::CursorStyle::Hand:
		cursor = ::LoadCursor(NULL, IDC_HAND);
		break;
	case platform::Window::CursorStyle::IBeam:
		cursor = ::LoadCursor(NULL, IDC_IBEAM);
		break;
	case platform::Window::CursorStyle::Cross:
		cursor = ::LoadCursor(NULL, IDC_CROSS);
		break;
	default:
		break;
	}
	if (cursor)
	{
		::SetClassLongPtr(
			m_hWnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
	}
}
platform::Position platform::Window::GetClientCursorPosition() const
{
	POINT point;
	if (::GetCursorPos(&point))
	{
		if (::ScreenToClient(m_hWnd, &point))
		{
			return { point.x, point.y };
		}
	}
	return { -1, -1 };
}

void platform::Window::RegisterMessageHandler(UINT message, MessageHandler handler)
{
	m_messageHandlers.insert_or_assign(message, handler);
}
void platform::Window::UnregisterMessageHandler(UINT message)
{
	auto it = m_messageHandlers.find(message);
	if (it != m_messageHandlers.end())
	{
		m_messageHandlers.erase(it);
	}
}

void platform::Window::Invalidate()
{
	if (m_hWnd)
	{
		::InvalidateRect(m_hWnd, NULL, false);
	}
}
int platform::Window::RunMessageLoop()
{
	MSG msg = {};
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}
