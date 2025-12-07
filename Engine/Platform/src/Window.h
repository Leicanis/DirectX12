#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace platform
{
	struct Size
	{
		long width, height;
	};
	struct Position
	{
		long x, y;
	};

	class Window
	{
	private:
		HINSTANCE m_hInstance{};
		HWND m_hWnd = nullptr;

		using MessageHandler = std::function<void(WPARAM, LPARAM)>;
		std::unordered_map<UINT, MessageHandler> m_messageHandlers;

		static std::unordered_set<std::wstring> s_registeredClasses;

	public:
		Window() = default;
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

	private:
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WndProc(
			HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	public:
		static bool RegisterWindowClass(HINSTANCE hInstance, UINT style, const std::wstring& className);

		bool Create(
			HINSTANCE hInstance,
			const std::wstring& title,
			int width, int height, DWORD style,
			const std::wstring& className, HWND parent = nullptr);
		bool ShowWindow(int cmdShow = SW_SHOWNORMAL);

		bool SetWindowTitle(const std::wstring& title);
		std::wstring GetWindowTitle() const;

		bool SetWindowSize(int width, int height);
		Size GetClientSize() const;

		enum class CursorStyle { Arrow, Hand, IBeam, Cross };
		void SetCursorStyle(CursorStyle style);
		Position GetClientCursorPosition() const;

		void RegisterMessageHandler(UINT message, MessageHandler handler);
		void UnregisterMessageHandler(UINT message);

		void Invalidate();
		int RunMessageLoop();
		HWND GetHandle() const { return m_hWnd; }

	};
}
