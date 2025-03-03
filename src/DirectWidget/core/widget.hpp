#pragma once

// base_widget.hpp: WidgetBase definition
// WidgetBase provides abstraction for all other widgets

// Standard headers

#include <functional>
#include <memory>

// Windows headers

#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>

// Local headers

#include "foundation.hpp"
#include "interop.hpp"
#include "element_base.hpp"
#include "property.hpp"
#include "resource.hpp"

namespace DirectWidget {

    class WidgetBase;
    using widget_ptr = std::shared_ptr<WidgetBase>;

    class LayoutContext {
    public:
        LayoutContext() : LayoutContext({ 0,0 }, { 0,0,0,0 }, { 0,0,0,0 }) {}

        LayoutContext(const SIZE_F& measure, const BOUNDS_F& constraints, const BOUNDS_F& margin, const widget_ptr& background_widget) :
            m_measure(measure), m_constraints(constraints), m_margin(margin), m_background_widget(background_widget) {
            m_layout_bounds = constraints;
        }

        LayoutContext(const SIZE_F& measure, const BOUNDS_F& constraints, const BOUNDS_F& margin) :
            LayoutContext(measure, constraints, margin, nullptr) {
        }

        void layout_child(const widget_ptr& child, const BOUNDS_F& constraints) const {
            layout_child(child, constraints, m_background_widget);
        }
        void layout_child(const widget_ptr& child, const BOUNDS_F& constraints, const widget_ptr& background) const;

        LayoutContext create_subcontext(const SIZE_F& measure, const BOUNDS_F& constraints, const BOUNDS_F& margin) const {
            return LayoutContext(measure, constraints, margin, m_background_widget);
        }

        LayoutContext create_subcontext(const SIZE_F& measure, const BOUNDS_F& constraints, const BOUNDS_F& margin, const widget_ptr& background_widget) const {
            return LayoutContext(measure, constraints, margin, background_widget);
        }

        const SIZE_F& measure() const { return m_measure; }
        const BOUNDS_F& constraints() const { return m_constraints; }
        const widget_ptr& background_widget() const { return m_background_widget; }

        BOUNDS_F& layout_bounds() { return m_layout_bounds; }

        const BOUNDS_F& layout_bounds() const { return m_layout_bounds; }

        BOUNDS_F render_bounds() const {
            return {
                m_layout_bounds.left + m_margin.left,
                m_layout_bounds.top + m_margin.top,
                m_layout_bounds.right - m_margin.right,
                m_layout_bounds.bottom - m_margin.bottom
            };
        }

    private:
        SIZE_F m_measure;
        BOUNDS_F m_constraints;
        BOUNDS_F m_layout_bounds;
        BOUNDS_F m_margin;
        widget_ptr m_background_widget;
    };

    class RenderContext {
    public:
        RenderContext(const Interop::com_ptr<ID2D1RenderTarget>& render_target, const BOUNDS_F& render_bounds)
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
        const Interop::com_ptr<ID2D1RenderTarget>& render_target() const { return m_render_target; }

    private:
        static const LogContext Logger;

        RenderContext(bool is_root, const Interop::com_ptr<ID2D1RenderTarget>& render_target, const BOUNDS_F& render_bounds)
            : m_is_root(is_root), m_render_target(render_target), m_render_bounds(render_bounds) {
            if (is_root) {
                m_render_target->BeginDraw();
            }
            auto bounds_rect = D2D1::RectF(render_bounds.left, render_bounds.top, render_bounds.right, render_bounds.bottom);
            render_target->PushAxisAlignedClip(bounds_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }

        bool m_is_root;
        Interop::com_ptr<ID2D1RenderTarget> m_render_target;
        BOUNDS_F m_render_bounds;
    };

    enum class WidgetAlignment {
        Start,
        Center,
        End,
        Stretch,
    };

    class WidgetBase : public ElementBase
    {
    public:
        // properties

        static property_ptr<SIZE_F> SizeProperty;
        static property_ptr<BOUNDS_F> MarginProperty;
        static property_ptr<WidgetAlignment> VerticalAlignmentProperty;
        static property_ptr<WidgetAlignment> HorizontalAlignmentProperty;

        static property_ptr<SIZE_F> MaxSizeProperty;
        static property_ptr<BOUNDS_F> ConstraintsProperty;

        SIZE_F size() const { return get_property<SIZE_F>(SizeProperty); }
        void set_size(const SIZE_F& size) { set_property<SIZE_F>(SizeProperty, size); }

        BOUNDS_F margin() const { return get_property<BOUNDS_F>(MarginProperty); }
        void set_margin(const BOUNDS_F& margin) { set_property<BOUNDS_F>(MarginProperty, margin); }

        WidgetAlignment vertical_alignment() const { return get_property<WidgetAlignment>(VerticalAlignmentProperty); }
        void set_vertical_alignment(WidgetAlignment alignment) { set_property<WidgetAlignment>(VerticalAlignmentProperty, alignment); }

        WidgetAlignment horizontal_alignment() const { return get_property<WidgetAlignment>(HorizontalAlignmentProperty); }
        void set_horizontal_alignment(WidgetAlignment alignment) { set_property<WidgetAlignment>(HorizontalAlignmentProperty, alignment); }

        const SIZE_F& maximum_size() const { return get_property<SIZE_F>(MaxSizeProperty); }
        void set_maximum_size(const SIZE_F& maximum_size) { set_property(MaxSizeProperty, maximum_size); }

        const BOUNDS_F& constraints() const { return get_property<BOUNDS_F>(ConstraintsProperty); }
        void set_constraints(const BOUNDS_F& constraints) { set_property<BOUNDS_F>(ConstraintsProperty, constraints); }

        // notification properties

        static property_base_ptr RenderTargetProperty;

        // resources

        static resource_ptr<SIZE_F> MeasureResource;
        static resource_ptr<LayoutContext> LayoutResource;
        static resource_ptr<BOUNDS_F> RenderBoundsResource;
        static Interop::com_resource_ptr<ID2D1Geometry> RenderGeometryResource;

        static resource_base_ptr RenderContentResource;

        static resource_ptr<float> ScaleResource;

        // layout

        void render_debug_layout(const Interop::com_ptr<ID2D1RenderTarget>& render_target) const;

        // rendering

        void attach_render_target(const Interop::com_ptr<ID2D1RenderTarget>& render_target);
        void detach_render_target();

        virtual void create_resources() { for_each_child([](WidgetBase* widget) { widget->create_resources(); }); }
        virtual void discard_resources() { for_each_child([](WidgetBase* widget) { widget->discard_resources(); }); }

        const Interop::com_ptr<ID2D1RenderTarget>& render_target() const { return m_render_target; }

        void issue_frame() { RenderContentResource->initialize_for(this); }
        void discard_frame() { 
            RenderContentResource->invalidate_for(this);
            for_each_child([](WidgetBase* child) {
                RenderContentResource->invalidate_for(child);
                });
        }

        // interaction

        D2D1_POINT_2F pixel_to_point(int x, int y) {
            auto& scale = ScaleResource->get_or_initialize_resource(this);
            return D2D1::Point2F(static_cast<float>(x) / scale, static_cast<float>(y) / scale);
        }

        D2D1_POINT_2U point_to_pixel(D2D1_POINT_2F point) {
            auto& scale = ScaleResource->get_or_initialize_resource(this);
            return D2D1::Point2U(static_cast<UINT>(point.x * scale), static_cast<UINT>(point.y * scale));
        }

        bool hit_test(D2D1_POINT_2F point);

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

        virtual void layout(LayoutContext& context) const;

    private:
        static const LogContext Logger;

        Interop::com_ptr<ID2D1RenderTarget> m_render_target = nullptr;

        class WidgetMeasureResource;
        class WidgetLayoutResource;
        class WidgetRenderBoundsResource;
        class WidgetRenderGeometryResource;
        class WidgetRenderContentResource;
        class WidgetRenderTargetProperty;

        friend LayoutContext;
    };
}