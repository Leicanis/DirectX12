#include "Exception.h"

#include <comdef.h>
#include <cstdlib>
#include <sstream>


void DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
{
    _com_error err(hr);
    std::wstring msg = err.ErrorMessage();

    std::wstringstream errorStream;
    errorStream << L"Error!\n\n"
        << L"Program: " << filename << "\n\n"
        << L"failed in: " << functionName << L"\n"
        << L"line: " << std::to_wstring(lineNumber) << L"\n\n"
        << L"error:\n    " << msg;

    int hMessageBox = MessageBoxEx(
        nullptr, errorStream.str().c_str(), L"Error",
        MB_ABORTRETRYIGNORE | MB_DEFBUTTON1 | MB_ICONERROR, 0);

    if (hMessageBox == IDABORT)
    {
        exit(EXIT_FAILURE);
    }
}
