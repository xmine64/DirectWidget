// app.cpp: Application implementation

#include <Windows.h>
#include <objbase.h>
#include <d2d1.h>
#include <dwrite.h>

#include "app.hpp"
#include "window.hpp"

using namespace DirectWidget;

// Initialize COM and device-independent resources
HRESULT Application::initialize()
{
    auto hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return hr;
    }

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2d);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_dwrite));
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

// Run the message loop
int Application::run_message_loop(Window& main_window, int nCmdShow)
{
    main_window.create();

    ShowWindow(main_window.window_handle(), nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

