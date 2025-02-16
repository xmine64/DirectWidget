#include <string>
#include <format>
#include <cassert>

#include <Windows.h>
#include <comdef.h>

#include "foundation.hpp"

using namespace DirectWidget;

void LogContext::log(PCWSTR message) const
{
    auto log = std::format(L"{}: {}\n", m_prefix, message);
    OutputDebugString(log.c_str());
}

void LogContext::log_error(HRESULT hr) const
{
#if _DEBUG
    assert(hr == S_OK);
#endif

    if (SUCCEEDED(hr)) return;

    _com_error error{ hr };
    log_error(error.ErrorMessage());
}

void LogContext::log_error(PCWSTR error) const
{
    auto log = std::format(L"{}: ERROR: {}\n", m_prefix, error);
    OutputDebugString(log.c_str());
}

void LogContext::fatal_exit(PCWSTR error) const
{
    log_error(error);
    FatalAppExit(0, error);
}

void LogContext::fatal_exit(HRESULT hr) const
{
#if _DEBUG
    assert(hr == S_OK);
#endif

    if (SUCCEEDED(hr)) return;

    _com_error error{ hr };
    fatal_exit(error.ErrorMessage());
}
