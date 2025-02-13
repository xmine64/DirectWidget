// window.hpp: Window definition
// Window provides a wrapper around win32 API to provide a render host window

#pragma once

#include <memory>

#include <windows.h>
#include <d2d1.h>

#include "foundation.hpp"
#include "widget.hpp"

namespace DirectWidget {


    class Window {
    public:

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        Window() : m_handle(nullptr) {}

        ~Window() {
            discard_device_resources();
        }

        virtual bool create();

        HWND window_handle() const { return m_handle; }
        float dpi() const { return m_dpi; }

        void set_root_widget(std::unique_ptr<WidgetBase> widget);

    protected:

        virtual PCWSTR class_name() const = 0;
        virtual UINT style() const { return WS_OVERLAPPEDWINDOW; }
        virtual PCWSTR title() const = 0;

        virtual bool on_size(int width, int height);
        virtual bool on_paint();
        virtual bool on_destroy() { return false; }
        virtual bool on_dpi_change(float dpi);
        virtual bool on_pointer_release(int x, int y);

        virtual LRESULT handle_message(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        HWND m_handle = NULL;
        float m_dpi = 1.0f;

        bool create_device_resources();
        void discard_device_resources();

        com_ptr<ID2D1HwndRenderTarget> m_render_target = nullptr;
        std::unique_ptr<WidgetBase> m_root_widget = nullptr;
    };

}