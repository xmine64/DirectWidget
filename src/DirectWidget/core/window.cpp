// window.cpp: Window implementation

#include <memory>
#include <string>

#include <Windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "foundation.hpp"
#include "window.hpp"
#include "app.hpp"
#include "widget.hpp"
#include "property.hpp"
#include "resource.hpp"

using namespace DirectWidget;

const LogContext Window::Logger{ NAMEOF(Window) };

property_ptr<widget_ptr> Window::RootWidgetProperty = make_property<widget_ptr>(nullptr);

LRESULT Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* window;
    if (uMsg == WM_NCCREATE) {
        auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = (Window*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

        //window->m_handle = hWnd;
    }
    else {
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (window != nullptr) {
        return window->handle_message(hWnd, uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

Window::Window() {
    register_property(RootWidgetProperty, m_root_widget);

    m_window_class = std::make_shared<Window::WindowClassResource>(this);
    m_window = std::make_shared<Window::WindowResource>(this);
    m_window->bind(m_window_class);

    m_client_rect = make_resource<RECT>([this]() {
        if (m_window->is_valid() == false) return RECT{}; // TODO: mark invalid

        auto& hwnd = m_window->get();

        RECT rc;
        GetClientRect(hwnd, &rc);
        return rc;
        });
    m_client_rect->bind(m_window);

    m_scale = make_resource<float>([this]() {
        if (m_window->is_valid() == false) return 1.0f; // TODO: mark invalid
        auto& hwnd = m_window->get();

        return static_cast<float>(GetDpiForWindow(hwnd)) / USER_DEFAULT_SCREEN_DPI;
        });
    m_scale->bind(m_window);
    m_scale->add_listener(std::make_shared<WindowDpiChangeListener>(this));

    m_render_target = std::make_shared<RenderTargetResource>(this);
    m_render_target->bind(m_window);
    m_render_target->bind(m_client_rect);
    m_render_target->add_listener(std::make_shared<RenderTargetSizeChangeListener>(this));
}

LRESULT Window::handle_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
    {
        m_client_rect->discard();
        InvalidateRect(m_window->get(), NULL, FALSE);
        return TRUE;
    }
    break;

    case WM_PAINT:
    {
        if (m_root_widget == nullptr) return FALSE;
        if (create_device_resources() == false) return FALSE;
        m_root_widget->render_content()->initialize();
        ValidateRect(m_window->get(), NULL);

        return TRUE;
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
        m_scale->discard();
        return TRUE;
    }
    break;

    case WM_MOUSEMOVE:
    {
        if (m_root_widget == nullptr) return FALSE;

        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (m_root_widget->handle_pointer_move(x, y)) {
            return TRUE;
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        if (m_root_widget == nullptr) return FALSE;

        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (m_root_widget->handle_pointer_press(x, y)) {
            return TRUE;
        }
    }
    break;

    case WM_LBUTTONUP:
    {
        if (m_root_widget == nullptr) return FALSE;

        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (m_root_widget->handle_pointer_release(x, y)) {
            return TRUE;
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Window::create_device_resources()
{
    if (m_root_widget == nullptr) return false;
    if (m_resource_created == true) return true;

    m_root_widget->attach_render_target(m_render_target->get());
    m_root_widget->attach_scale(m_scale);
    m_root_widget->create_resources();

    auto render_target_size = m_render_target->get()->GetSize();
    m_root_widget->set_maximum_size(SIZE_F{ render_target_size.width, render_target_size.height });

    auto view_port = BOUNDS_F{ 0,0,render_target_size.width, render_target_size.height };
    BOUNDS_F layout_bounds;
    BOUNDS_F render_bounds;
    m_root_widget->layout(view_port, layout_bounds, render_bounds);
    m_root_widget->finalize_layout(render_bounds);

    m_resource_created = true;
    return true;
}

void Window::discard_device_resources()
{
    m_root_widget->discard_resources();
    m_root_widget->detach_render_target();
    m_root_widget->render_content()->discard();
    m_resource_created = false;
}

void Window::WindowClassResource::initialize() {
    if (is_valid() == true) return;

    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = WindowProc,
        .hInstance = GetModuleHandle(nullptr),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = m_owner->class_name(),
    };
    m_atom = RegisterClassEx(&wc);
    if (m_atom != 0) mark_valid();
}

void Window::WindowResource::initialize() {
    if (is_valid() == true) return;

    m_owner->m_window_class->initialize();

    m_handle = CreateWindowEx(
        0,
        m_owner->class_name(),
        m_owner->title(),
        m_owner->style(),
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        m_owner
    );

    if (m_handle != NULL) mark_valid();
}

void Window::RenderTargetResource::initialize() {
    if (is_valid() == true) return;

    auto& hwnd = m_owner->m_window->get();
    auto& rc = m_owner->m_client_rect->get();

    auto size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top);

    auto& factory = Application::instance()->d2d();
    auto hr = factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, size),
        &m_render_target);
    Logger.at(NAMEOF(create_device_resources)).at(NAMEOF(factory->CreateHwndRenderTarget)).fatal_exit(hr);

    mark_valid();
}
