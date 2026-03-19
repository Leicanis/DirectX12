#pragma once

#include <string>
#include <windows.h>

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                                          \
{                                                                                                 \
    HRESULT hr__ = (x);                                                                           \
    if(FAILED(hr__)) { DxException(hr__, AnsiToWString(#x), AnsiToWString(__FILE__), __LINE__); } \
}
#endif)

void DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);
inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}
