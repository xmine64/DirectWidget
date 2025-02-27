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

namespace DirectWidget {

    typedef struct {

        BOUNDS_F render_bounds;

        com_ptr<ID2D1Geometry> geometry;

    } LAYOUT_STATE;

    enum WIDGET_ALIGNMENT {
        WIDGET_ALIGNMENT_START,
        WIDGET_ALIGNMENT_CENTER,
        WIDGET_ALIGNMENT_END,
        WIDGET_ALIGNMENT_STRETCH,
    };

    class WidgetBase : public PropertyOwnerBase
    {
    public:
        
        // properties

        static property_ptr<SIZE_F> SizeProperty;
        static property_ptr<BOUNDS_F> MarginProperty;
        static property_ptr<WIDGET_ALIGNMENT> VerticalAlignmentProperty;
        static property_ptr<WIDGET_ALIGNMENT> HorizontalAlignmentProperty;

        // Not an actual property, but can be used as dependency for resources
        static property_base_ptr RenderTargetProperty;
        static property_base_ptr LayoutProperty;

        SIZE_F size() const { return get_property<SIZE_F>(SizeProperty); }
        void set_size(const SIZE_F& size) { set_property<SIZE_F>(SizeProperty, size); }

        BOUNDS_F margin() const { return get_property<BOUNDS_F>(MarginProperty); }
        void set_margin(const BOUNDS_F& margin) { set_property<BOUNDS_F>(MarginProperty, margin); }

        WIDGET_ALIGNMENT vertical_alignment() const { return get_property<WIDGET_ALIGNMENT>(VerticalAlignmentProperty); }
        void set_vertical_alignment(WIDGET_ALIGNMENT alignment) { set_property<WIDGET_ALIGNMENT>(VerticalAlignmentProperty, alignment); }

        WIDGET_ALIGNMENT horizontal_alignment() const { return get_property<WIDGET_ALIGNMENT>(HorizontalAlignmentProperty); }
        void set_horizontal_alignment(WIDGET_ALIGNMENT alignment) { set_property<WIDGET_ALIGNMENT>(HorizontalAlignmentProperty, alignment); }

        // layout

        virtual void layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) const;
        void finalize_layout(const BOUNDS_F& render_bounds);

        virtual SIZE_F measure(const SIZE_F& available_size) const { return size_min(m_size, available_size); }

        void render_debug_layout(const com_ptr<ID2D1RenderTarget>& render_target) const;

        // rendering

        void attach_render_target(const com_ptr<ID2D1RenderTarget>& render_target);
        void detach_render_target();
        const com_ptr<ID2D1RenderTarget>& render_target() const { return m_render_target; }

        virtual void create_resources() { for_each_child([](WidgetBase* widget) { widget->create_resources(); }); }
        virtual void discard_resources() { for_each_child([](WidgetBase* widget) { widget->discard_resources(); }); }

        BOUNDS_F render_bounds() const { return m_layout.render_bounds; }

        void render() const;

        // interaction

        void update_dpi(float dpi) { m_dpi = dpi; for_each_child([dpi](WidgetBase* widget) { widget->update_dpi(dpi); }); }

        D2D1_POINT_2F pixel_to_point(int x, int y) const {
            return D2D1::Point2F(static_cast<float>(x) / m_dpi, static_cast<float>(y) / m_dpi);
        }

        D2D1_POINT_2U point_to_pixel(D2D1_POINT_2F point) const {
            return D2D1::Point2U(static_cast<UINT>(point.x * m_dpi), static_cast<UINT>(point.y * m_dpi));
        }

        bool hit_test(D2D1_POINT_2F point) const;

        virtual bool handle_pointer_hover(D2D1_POINT_2F point) { return false; }
        virtual bool handle_pointer_press(D2D1_POINT_2F point) { return false; }
        virtual bool handle_pointer_release(D2D1_POINT_2F point) { return false; }

    protected:

        WidgetBase() {
            register_property(SizeProperty, m_size);
            register_property(MarginProperty, m_margin);
            register_property(VerticalAlignmentProperty, m_vertical_alignment);
            register_property(HorizontalAlignmentProperty, m_horizontal_alignment);
        }

        virtual void for_each_child(std::function<void(WidgetBase*)> callback) const {}
        
        virtual void on_layout_finalized(const BOUNDS_F& render_bounds) {}

        virtual void on_render() const {}

    private:
        com_ptr<ID2D1RenderTarget> m_render_target = nullptr;

        float m_dpi = 1.0f;
        
        LAYOUT_STATE m_layout{};

        SIZE_F m_size;
        BOUNDS_F m_margin;
        WIDGET_ALIGNMENT m_vertical_alignment;
        WIDGET_ALIGNMENT m_horizontal_alignment;
    };
}