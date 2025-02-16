// window.cpp: Window implementation

#include <memory>

#include <Windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "foundation.hpp"
#include "window.hpp"
#include "app.hpp"
#include "widget.hpp"

using namespace DirectWidget;

const LogContext Window::m_log{ NAMEOF(Window) };

LRESULT Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* window;
    if (uMsg == WM_NCCREATE) {
        auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = (Window*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

        window->m_handle = hWnd;
    }
    else {
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (window != nullptr) {
        return window->handle_message(uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

bool Window::create() {
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = WindowProc,
        .hInstance = GetModuleHandle(nullptr),
        .hCursor = LoadCursor(NULL, IDI_APPLICATION),
        .lpszClassName = class_name(),
    };

    RegisterClassEx(&wc);

    m_handle = CreateWindowEx(
        0,
        class_name(),
        title(),
        style(),
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        this
    );

    m_dpi = static_cast<float>(GetDpiForWindow(m_handle)) / USER_DEFAULT_SCREEN_DPI;
    if (m_root_widget != nullptr) {
        m_root_widget->update_dpi(m_dpi);
    }

    return m_handle != nullptr;
}

LRESULT Window::handle_message(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
    {
        auto width = LOWORD(lParam), height = HIWORD(lParam);
        if (on_size(width, height)) {
            return TRUE;
        }
    }
    break;

    case WM_PAINT:
    {
        if (on_paint()) {
            return TRUE;
        }
    }
    break;

    case WM_DESTROY:
    {
        if (on_destroy()) {
            return TRUE;
        }
    }
    break;

    case WM_DPICHANGED:
    {
        auto dpi = static_cast<float>(GetDpiForWindow(m_handle)) / USER_DEFAULT_SCREEN_DPI;
        if (on_dpi_change(dpi)) {
            return TRUE;
        }
    }
    break;

    case WM_LBUTTONUP:
    {
        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (on_pointer_release(x, y)) {
            return TRUE;
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(this->m_handle, uMsg, wParam, lParam);
}

void Window::set_root_widget(const std::shared_ptr<WidgetBase>& widget)
{
    m_root_widget = widget;
    m_root_widget->update_dpi(m_dpi);

    if (m_render_target != NULL)
    {
        auto render_target_size = m_render_target->GetSize();

        auto view_port = BOUNDS_F{ 0,0,render_target_size.width, render_target_size.height };
        BOUNDS_F layout_bounds;
        BOUNDS_F render_bounds;
        m_root_widget->layout(view_port, layout_bounds, render_bounds);
        m_root_widget->finalize_layout(render_bounds);
    }
}

bool Window::on_size(int width, int height)
{
    if (m_render_target != NULL)
    {
        auto size = D2D1::SizeU(width, height);
        m_render_target->Resize(size);

        if (m_root_widget != nullptr)
        {
            auto render_target_size = m_render_target->GetSize();

            auto view_port = BOUNDS_F{ 0,0,render_target_size.width, render_target_size.height };
            BOUNDS_F render_bounds;
            BOUNDS_F layout_bounds;
            m_root_widget->layout(view_port, layout_bounds, render_bounds);
            m_root_widget->finalize_layout(render_bounds);
        }

        InvalidateRect(window_handle(), nullptr, FALSE);
    }
    return true;
}

bool Window::on_paint()
{
    if (create_device_resources() == false) {
        return false;
    }

    m_render_target->BeginDraw();

    m_render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));

    if (m_root_widget != nullptr)
    {
        m_root_widget->render();

#ifdef _DEBUG

        if (Application::instance()->is_debug())
            m_root_widget->render_debug_layout(m_render_target);

#endif // DEBUG
    }

    auto hr = m_render_target->EndDraw();
    if (FAILED(hr))
    {
        if (hr == D2DERR_RECREATE_TARGET)
        {
            discard_device_resources();
            return true;
        }
        else
        {
            m_log.at(NAMEOF(on_paint)).fatal_exit(hr);
            return false;
        }
    }

    ValidateRect(window_handle(), nullptr);

    return true;
}

bool Window::on_dpi_change(float dpi) {
    m_dpi = dpi;

    m_root_widget->update_dpi(dpi);

    InvalidateRect(window_handle(), NULL, TRUE);

    return true;
}

bool Window::on_pointer_release(int x, int y)
{
    if (m_render_target == NULL || m_root_widget == nullptr)
        return false;
    auto point = m_root_widget->pixel_to_point(x, y);
    return m_root_widget->handle_pointer_release(point);
}

bool Window::create_device_resources()
{
    if (m_render_target != nullptr) {
        return true;
    }

    RECT rc;
    GetClientRect(window_handle(), &rc);

    auto size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top);

    auto& factory = Application::instance()->d2d();

    auto hr = factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(window_handle(), size),
        &m_render_target);
    if (FAILED(hr)) {
        m_log.at(NAMEOF(create_device_resources)).at(NAMEOF(factory->CreateHwndRenderTarget)).fatal_exit(hr);
        return false;
    }

    if (m_root_widget != nullptr)
    {
        m_root_widget->attach_render_target(m_render_target);
        m_root_widget->create_resources();

        auto render_target_size = m_render_target->GetSize();

        auto view_port = BOUNDS_F{ 0,0,render_target_size.width, render_target_size.height };
        BOUNDS_F layout_bounds;
        BOUNDS_F render_bounds;
        m_root_widget->layout(view_port, layout_bounds, render_bounds);
        m_root_widget->finalize_layout(render_bounds);
    }

    return true;
}

void Window::discard_device_resources()
{
    m_root_widget->discard_resources();
    m_root_widget->detach_render_target();
    m_render_target = nullptr;
}
