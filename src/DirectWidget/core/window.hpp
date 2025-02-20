// window.hpp: Window definition
// Window provides a wrapper around win32 API to provide a render host window

#pragma once

#include <memory>

#include <windows.h>
#include <d2d1.h>

#include "foundation.hpp"
#include "widget.hpp"
#include "property.hpp"
#include "resource.hpp"

namespace DirectWidget {

    class Window : public ElementBase {
    public:

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // properties

        static property_ptr<widget_ptr> RootWidgetProperty;

        const widget_ptr& root_widget() { return get_property<widget_ptr>(RootWidgetProperty); }
        void set_root_widget(const widget_ptr& widget) { set_property<widget_ptr>(RootWidgetProperty, widget); }

        Window();
        ~Window() { discard_device_resources(); }

        void show(int nCmdShow) { ShowWindow(m_window->get(), nCmdShow); }
        void show() { show(SWP_SHOWWINDOW); }
        void close() { m_window->discard(); }

    protected:

        virtual PCWSTR class_name() const = 0;
        virtual UINT style() const { return WS_OVERLAPPEDWINDOW; }
        virtual PCWSTR title() const = 0;

        virtual bool on_destroy() { return false; }

        virtual LRESULT handle_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:

        static const LogContext Logger;

        bool create_device_resources();
        void discard_device_resources();

        resource_ptr<ATOM> m_window_class;
        resource_ptr<HWND> m_window;
        resource_ptr<RECT> m_client_rect;
        resource_ptr<float> m_scale;
        com_resource_ptr<ID2D1RenderTarget> m_render_target;

        bool m_resource_created = false;
        std::shared_ptr<WidgetBase> m_root_widget = nullptr;

        class WindowClassResource : public TypedResourceBase<ATOM> {
        public:
            WindowClassResource(Window* owner) : m_owner(owner) {}

            void initialize() override;

            void discard() override {
                if (is_valid() == false) return;

                UnregisterClass(m_owner->class_name(), GetModuleHandle(nullptr));
                mark_invalid();
            }

        protected:
            const ATOM& get_internal() const override { return m_atom; };

        private:
            Window* m_owner;

            ATOM m_atom = 0;
        };

        class WindowResource : public TypedResourceBase<HWND> {
        public:
            WindowResource(Window* owner) : m_owner(owner), m_handle(NULL) {}

            void initialize() override;

            void discard() override {
                if (is_valid() == false) return;

                DestroyWindow(m_handle);
                m_handle = NULL;
                mark_invalid();
            }

        protected:
            const HWND& get_internal() const override { return m_handle; }

        private:
            Window* m_owner;

            HWND m_handle;
        };

        class RenderTargetResource : public TypedResourceBase<com_ptr<ID2D1RenderTarget>> {
        public:
            RenderTargetResource(Window*owner) : m_owner(owner) {}

            void initialize() override;

            void discard() override {
                if (is_valid() == false) return;
                m_render_target = nullptr;
                mark_invalid();
            }
            
            bool try_update(const resource_base_ptr& resource) override {
                if (resource != m_owner->m_client_rect) return false;

                auto& rc = m_owner->m_client_rect->get();

                auto size = D2D1::SizeU(
                    rc.right - rc.left,
                    rc.bottom - rc.top);

                m_render_target->Resize(size);

                return true;
            }

            const com_ptr<ID2D1RenderTarget>& get_internal() const override { return m_render_target; }

        private:
            Window* m_owner;
            com_ptr<ID2D1HwndRenderTarget> m_render_target;
        };

        class RenderTargetSizeChangeListener : public ResourceListenerBase {
        public:
            RenderTargetSizeChangeListener(Window* owner)
                : m_owner(owner) {
            }

            void on_resource_updated(ResourceBase* resource) override {
                layout_widget();
            }

            void on_resource_initialized(ResourceBase* resource) override {
                layout_widget();
            }

            void layout_widget();

        private:
            Window* m_owner;
        };

        class WindowDpiChangeListener : public ResourceListenerBase {
        public:
            WindowDpiChangeListener(Window* owner)
                : m_owner(owner) {
            }
            void on_resource_invalidated(ResourceBase* resource) override {
                if (m_owner->m_window->is_valid() == false) return;
                if (m_owner->m_render_target->is_valid() == false) return;
                auto dpi = m_owner->m_scale->get();
                m_owner->m_render_target->get()->SetDpi(dpi, dpi);
            }
        private:
            Window* m_owner;
        };
    };

}