// window.hpp: Window definition
// Window provides a wrapper around win32 API to provide a render host window

#pragma once

#include <memory>

#include <windows.h>
#include <d2d1.h>

#include "foundation.hpp"
#include "element_base.hpp"
#include "property.hpp"
#include "resource.hpp"
#include "interop.hpp"
#include "widget.hpp"

namespace DirectWidget {

    class Window : public ElementBase {
    public:

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // properties

        static property_ptr<PCWSTR> ClassNameProperty;
        static property_ptr<PCWSTR> TitleProperty;
        static property_ptr<UINT> StyleProperty;
        static property_ptr<widget_ptr> RootWidgetProperty;

        const widget_ptr& root_widget() { return RootWidgetProperty->get_value(this); }
        void set_root_widget(const widget_ptr& widget) { 
            auto& old_widget = root_widget();
            if (old_widget != nullptr) {
                detach_child(old_widget);
            }
            RootWidgetProperty->set_value(this, widget); 
            register_child(widget);
        }

        // resources

        static resource_ptr<ATOM> ClassResource;
        static resource_ptr<HWND> WindowResource;
        static resource_ptr<RECT> ClientRectResource;
        static resource_ptr<float> ScaleResource;
        static Interop::com_resource_ptr<ID2D1HwndRenderTarget> RenderTargetResource;

        const HWND& window_handle() const { return WindowResource->get_resource(this); }
        const RECT& client_rect() const { return ClientRectResource->get_resource(this); }
        const float& scale() const { return ScaleResource->get_resource(this); }
        const Interop::com_ptr<ID2D1RenderTarget>& render_target() const { return RenderTargetResource->get_resource(this); }

        Window();
        ~Window() { discard_device_resources(); }

        void show(int nCmdShow) { ShowWindow(WindowResource->get_or_initialize_resource(this), nCmdShow); }
        void show() { show(SWP_SHOWWINDOW); }
        void close() { WindowResource->invalidate_for(this); }

    protected:
        virtual bool on_destroy() { return false; }

        virtual LRESULT handle_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        static const LogContext Logger;

        class WidgetRenderContentListener;

        static std::shared_ptr<WidgetRenderContentListener> RenderContentListener;

        bool create_device_resources();
        void discard_device_resources();

        bool m_resource_created = false;
    };
}