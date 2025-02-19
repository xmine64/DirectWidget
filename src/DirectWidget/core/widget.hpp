// base_widget.hpp: WidgetBase definition
// WidgetBase provides abstraction for all other widgets

#pragma once

// Standard headers

#include <functional>
#include <memory>

// Windows headers

#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>

// Local headers

#include "foundation.hpp"
#include "property.hpp"
#include "resource.hpp"

namespace DirectWidget {

    typedef struct {

        BOUNDS_F render_bounds;

        com_ptr<ID2D1Geometry> geometry;

    } LAYOUT_STATE;

    class RenderContext {
    public:
        RenderContext(const com_ptr<ID2D1RenderTarget>& render_target, const BOUNDS_F& render_bounds)
            : RenderContext(true, render_target, render_bounds) {

        }

        ~RenderContext() {
            m_render_target->PopAxisAlignedClip();
            if (m_is_root)
            {
                auto hr = m_render_target->EndDraw();
                Logger.at(NAMEOF(~RenderContext)).at(NAMEOF(ID2D1RenderTarget::EndDraw)).log_error(hr);
            }
            else {
                auto hr = m_render_target->Flush();
                Logger.at(NAMEOF(~RenderContext)).at(NAMEOF(ID2D1RenderTarget::Flush)).log_error(hr);
            }
        }

        RenderContext(RenderContext&) = delete;
        RenderContext(RenderContext&&) = delete;

        RenderContext create_subcontext(const BOUNDS_F& render_bounds) const {
            return RenderContext(false, m_render_target, render_bounds);
        }

        const BOUNDS_F& render_bounds() const { return m_render_bounds; }
        const com_ptr<ID2D1RenderTarget>& render_target() const { return m_render_target; }

    private:
        static const LogContext Logger;

        RenderContext(bool is_root, const com_ptr<ID2D1RenderTarget>& render_target, const BOUNDS_F& render_bounds)
            : m_is_root(is_root), m_render_target(render_target), m_render_bounds(render_bounds) {
            if (is_root) {
                m_render_target->BeginDraw();
            }
            auto bounds_rect = D2D1::RectF(render_bounds.left, render_bounds.top, render_bounds.right, render_bounds.bottom);
            render_target->PushAxisAlignedClip(bounds_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }

        bool m_is_root;
        com_ptr<ID2D1RenderTarget> m_render_target;
        BOUNDS_F m_render_bounds;
    };

    enum WIDGET_ALIGNMENT {
        WIDGET_ALIGNMENT_START,
        WIDGET_ALIGNMENT_CENTER,
        WIDGET_ALIGNMENT_END,
        WIDGET_ALIGNMENT_STRETCH,
    };

    class WidgetBase;
    using widget_ptr = std::shared_ptr<WidgetBase>;

    class WidgetBase : public PropertyOwnerBase
    {
    public:

        // properties

        static property_ptr<SIZE_F> SizeProperty;
        static property_ptr<BOUNDS_F> MarginProperty;
        static property_ptr<WIDGET_ALIGNMENT> VerticalAlignmentProperty;
        static property_ptr<WIDGET_ALIGNMENT> HorizontalAlignmentProperty;

        static property_ptr<SIZE_F> MaxSizeProperty;
        static property_ptr<BOUNDS_F> ConstraintsProperty;

        SIZE_F size() const { return get_property<SIZE_F>(SizeProperty); }
        void set_size(const SIZE_F& size) { set_property<SIZE_F>(SizeProperty, size); }

        BOUNDS_F margin() const { return get_property<BOUNDS_F>(MarginProperty); }
        void set_margin(const BOUNDS_F& margin) { set_property<BOUNDS_F>(MarginProperty, margin); }

        WIDGET_ALIGNMENT vertical_alignment() const { return get_property<WIDGET_ALIGNMENT>(VerticalAlignmentProperty); }
        void set_vertical_alignment(WIDGET_ALIGNMENT alignment) { set_property<WIDGET_ALIGNMENT>(VerticalAlignmentProperty, alignment); }

        WIDGET_ALIGNMENT horizontal_alignment() const { return get_property<WIDGET_ALIGNMENT>(HorizontalAlignmentProperty); }
        void set_horizontal_alignment(WIDGET_ALIGNMENT alignment) { set_property<WIDGET_ALIGNMENT>(HorizontalAlignmentProperty, alignment); }

        const SIZE_F& maximum_size() const { return get_property<SIZE_F>(MaxSizeProperty); }
        void set_maximum_size(const SIZE_F& maximum_size) { set_property(MaxSizeProperty, maximum_size); }

        const BOUNDS_F& constraints() const { return get_property<BOUNDS_F>(ConstraintsProperty); }
        void set_constraints(const BOUNDS_F& constraints) { set_property<BOUNDS_F>(ConstraintsProperty, constraints); }

        // notification properties

        /// <summary>
        /// RenderTargetProperty is a notification property that is used to notify the widget that the render target has changed.
        /// </summary>
        static property_base_ptr RenderTargetProperty;

        // layout

        const resource_ptr<SIZE_F> measure_resource() const { return m_measure; }
        const resource_ptr<BOUNDS_F> layout_bounds_resource() const { return m_layout_bounds; }
        const resource_ptr<BOUNDS_F> render_bounds_resource() const { return m_render_bounds; }

        void finalize_layout(const BOUNDS_F& render_bounds);

        void render_debug_layout(const com_ptr<ID2D1RenderTarget>& render_target) const;

        // rendering

        void attach_render_target(const com_ptr<ID2D1RenderTarget>& render_target);
        void detach_render_target();
        const resource_base_ptr& render_content() const { return m_render_content; }

        virtual void create_resources() { for_each_child([](WidgetBase* widget) { widget->create_resources(); }); }
        virtual void discard_resources() { for_each_child([](WidgetBase* widget) { widget->discard_resources(); }); }

        const com_ptr<ID2D1RenderTarget>& render_target() const { return m_render_target; }

        // interaction

        void attach_scale(const resource_ptr<float>& scale) {
            m_scale = scale;
            for_each_child([&scale](WidgetBase* child) {
                child->attach_scale(scale);
                });
        }
        
        D2D1_POINT_2F pixel_to_point(int x, int y) const {
            auto& scale = m_scale->get();
            return D2D1::Point2F(static_cast<float>(x) / scale, static_cast<float>(y) / scale);
        }

        D2D1_POINT_2U point_to_pixel(D2D1_POINT_2F point) const {
            auto& scale = m_scale->get();
            return D2D1::Point2U(static_cast<UINT>(point.x * scale), static_cast<UINT>(point.y * scale));
        }

        bool hit_test(D2D1_POINT_2F point) const;

        virtual bool handle_pointer_hover(D2D1_POINT_2F point) { return false; }
        virtual bool handle_pointer_press(D2D1_POINT_2F point) { return false; }
        virtual bool handle_pointer_release(D2D1_POINT_2F point) { return false; }

        bool handle_pointer_move(int x, int y) {
            return false; // TODO
        }

        bool handle_pointer_press(int x, int y) {
            return handle_pointer_press(pixel_to_point(x, y));
        }

        bool handle_pointer_release(int x, int y) {
            return handle_pointer_release(pixel_to_point(x, y));
        }

    protected:

        WidgetBase();

        virtual void for_each_child(std::function<void(WidgetBase*)> callback) const {}

        virtual void render(const RenderContext& context) const {}

        virtual SIZE_F measure(const SIZE_F& maximum_size) const { return { 0,0 }; }

        virtual void layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds);

    private:
        static const LogContext Logger;

        SIZE_F m_max_size;
        resource_ptr<SIZE_F> m_measure;
        
        BOUNDS_F m_constraints;
        resource_ptr<BOUNDS_F> m_layout_bounds;
        resource_ptr<BOUNDS_F> m_render_bounds;

        resource_ptr<float> m_scale;

        com_ptr<ID2D1RenderTarget> m_render_target = nullptr;
        resource_base_ptr m_render_content;

        LAYOUT_STATE m_layout{};

        SIZE_F m_size;
        BOUNDS_F m_margin;
        WIDGET_ALIGNMENT m_vertical_alignment;
        WIDGET_ALIGNMENT m_horizontal_alignment;

        class RenderContentResource : public ResourceBase {
        public:
            RenderContentResource(WidgetBase* widget) : m_widget(widget) {}
            void initialize() override;
            void initialize_with_context(const RenderContext& render_context);
            void discard() override { mark_invalid(); }
        private:
            WidgetBase* m_widget;
        };
    };
}