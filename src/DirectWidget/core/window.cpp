// window.cpp: Window implementation

#include <memory>

#include <Windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "foundation.hpp"
#include "element_base.hpp"
#include "property.hpp"
#include "resource.hpp"
#include "interop.hpp"
#include "window.hpp"
#include "app.hpp"
#include "widget.hpp"

using namespace DirectWidget;

class Win32WindowClassResource : public BasicTypeResource<ATOM> {
protected:
    bool initialize(const ElementBase* owner, ATOM& resource) override {
        if (is_valid(owner) == true) return true;

        WNDCLASSEX wc = {
            .cbSize = sizeof(WNDCLASSEX),
            .lpfnWndProc = Window::WindowProc,
            .hInstance = GetModuleHandle(nullptr),
            .hCursor = LoadCursor(NULL, IDC_ARROW),
            .lpszClassName = Window::ClassNameProperty->get_value(owner),
        };
        resource = RegisterClassEx(&wc);
        return resource != 0;
    }

    void discard(const ElementBase* owner, ATOM& resource) override {
        UnregisterClass(Window::ClassNameProperty->get_value(owner), GetModuleHandle(nullptr));
    }
};

class Win32WindowResource : public BasicTypeResource<HWND> {
protected:
    bool initialize(const ElementBase* owner, HWND& resource) override {
        if (is_valid(owner) == true) return true;

        Window::ClassResource->initialize_for(owner);

        resource = CreateWindowEx(
            0,
            Window::ClassNameProperty->get_value(owner),
            Window::TitleProperty->get_value(owner),
            Window::StyleProperty->get_value(owner),
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL,
            NULL,
            GetModuleHandle(NULL),
            const_cast<void*>(reinterpret_cast<const void*>(owner)) // FIXME
        );
        return resource != NULL;
    }

    void discard(const ElementBase* owner, HWND& resource) override {
        if (is_valid(owner) == false) return;

        DestroyWindow(resource);
        resource = NULL;
    }
};

class Win32ClientRectResource : public BasicTypeResource<RECT> {
protected:
    bool initialize(const ElementBase* owner, RECT& resource) override {
        if (is_valid(owner) == true) return true;
        if (Window::WindowResource->is_valid(owner) == false) return false;

        auto& hwnd = Window::WindowResource->get_resource(owner);
        return GetClientRect(hwnd, &resource);
    }
};

class Win32ScaleResource : public BasicTypeResource<float> {
protected:
    bool initialize(const ElementBase* owner, float& resource) override {
        if (is_valid(owner) == true) return true;
        if (Window::WindowResource->is_valid(owner) == false) return false;

        auto& hwnd = Window::WindowResource->get_resource(owner);
        resource = static_cast<float>(GetDpiForWindow(hwnd)) / USER_DEFAULT_SCREEN_DPI;
        return true;
    }
};

class HwndRenderTargetResource : public Interop::ComResource<ID2D1HwndRenderTarget> {
public:
    HRESULT initialize(const ElementBase* owner, Interop::com_ptr<ID2D1HwndRenderTarget>& resource) override {
        if (is_valid(owner) == true) return S_OK;

        auto& hwnd = Window::WindowResource->get_or_initialize_resource(owner);
        auto& rc = Window::ClientRectResource->get_or_initialize_resource(owner);

        auto size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        auto& factory = Application::instance()->d2d();
        return factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
            &resource);
    }

    // TODO: Resize when client rect changed
};


class Window::WidgetRenderContentListener : public DependencyListenerBase {
public:
    void register_window(const ElementBase* owner, Window* window) {
        m_window[owner] = window;
    }

    void remove_window_for(const ElementBase* owner) {
        m_window.erase(owner);
    }

    void on_dependency_updated(const ElementBase* owner, const NotificationArgument& arg) override {
        auto window = m_window.find(owner);
        if (window == m_window.end()) {
            return;
        }
        auto& hwnd = Window::WindowResource->get_resource(window->second);

        if (arg.notification_type() == NotificationType::Initialized) {
            ValidateRect(hwnd, NULL);
        }
        else {
            InvalidateRect(hwnd, NULL, FALSE);
        }
    }

private:
    std::unordered_map<const ElementBase*, Window*> m_window;
};

const LogContext Window::Logger{ NAMEOF(Window) };

property_ptr<PCWSTR> Window::ClassNameProperty = make_property<PCWSTR>(NAMEOF(DirectWidget::Window));
property_ptr<PCWSTR> Window::TitleProperty = make_property<PCWSTR>(NAMEOF(DirectWidget::Window));
property_ptr<UINT> Window::StyleProperty = make_property<UINT>(WS_OVERLAPPEDWINDOW);
property_ptr<widget_ptr> Window::RootWidgetProperty = make_property<widget_ptr>(nullptr);

resource_ptr<ATOM> Window::ClassResource = std::make_shared<Win32WindowClassResource>();
resource_ptr<HWND> Window::WindowResource = std::make_shared<Win32WindowResource>();
resource_ptr<RECT> Window::ClientRectResource = std::make_shared<Win32ClientRectResource>();
resource_ptr<float> Window::ScaleResource = std::make_shared<Win32ScaleResource>();
Interop::com_resource_ptr<ID2D1HwndRenderTarget> Window::RenderTargetResource = std::make_shared<HwndRenderTargetResource>();

std::shared_ptr<Window::WidgetRenderContentListener> Window::RenderContentListener = []() {
    auto listener = std::make_shared<WidgetRenderContentListener>();
    WidgetBase::RenderContentResource->add_listener(listener);
    return listener;
    }();

LRESULT Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* window;
    if (uMsg == WM_NCCREATE) {
        auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = (Window*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        static_pointer_cast<BasicTypeResource<HWND>>(WindowResource)->update_resource(window, hWnd);
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
    register_dependency(ClassNameProperty);
    register_dependency(TitleProperty);
    register_dependency(StyleProperty);
    register_dependency(RootWidgetProperty);

    register_dependency(ClassResource);
    register_dependency(WindowResource);
    register_dependency(ClientRectResource);
    register_dependency(ScaleResource);
    register_dependency(RenderTargetResource);
}

LRESULT Window::handle_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
    {
        ClientRectResource->invalidate_for(this);
        return TRUE;
    }
    break;

    case WM_PAINT:
    {
        if (root_widget() == nullptr) return FALSE;
        if (create_device_resources() == false) return FALSE;
        root_widget()->issue_frame();

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
        ScaleResource->invalidate_for(this);
        return TRUE;
    }
    break;

    case WM_MOUSEMOVE:
    {
        if (root_widget() == nullptr) return FALSE;

        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (root_widget()->handle_pointer_move(x, y)) {
            return TRUE;
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        if (root_widget() == nullptr) return FALSE;

        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (root_widget()->handle_pointer_press(x, y)) {
            return TRUE;
        }
    }
    break;

    case WM_LBUTTONUP:
    {
        if (root_widget() == nullptr) return FALSE;

        auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (root_widget()->handle_pointer_release(x, y)) {
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
    if (root_widget() == nullptr) return false;
    if (m_resource_created == true) return true;

    RenderContentListener->register_window(root_widget().get(), this);

    auto& render_target = RenderTargetResource->get_or_initialize_resource(this);
    root_widget()->attach_render_target(render_target);
    root_widget()->create_resources();

    auto render_target_size = render_target->GetSize();
    root_widget()->set_maximum_size(SIZE_F{ render_target_size.width, render_target_size.height });
    root_widget()->set_constraints(BOUNDS_F{ 0,0,render_target_size.width, render_target_size.height });

    m_resource_created = true;
    return true;
}

void Window::discard_device_resources()
{
    RenderContentListener->remove_window_for(root_widget().get());

    root_widget()->discard_resources();
    root_widget()->detach_render_target();
    root_widget()->discard_frame();
    m_resource_created = false;
}
