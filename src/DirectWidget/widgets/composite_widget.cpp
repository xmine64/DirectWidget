// composite_widget.cpp: Composite widget implementation

#include <memory>

#include <Windows.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"
#include "composite_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

collection_property_ptr<std::shared_ptr<WidgetBase>> CompositeWidget::ChildrenProperty = make_collection<std::shared_ptr<WidgetBase>>();

SIZE_F CompositeWidget::measure(const SIZE_F& available_size) const
{
    auto result = WidgetBase::measure(available_size);

    for (auto& child : m_children) {
        child->set_maximum_size(available_size);
        auto size = child->measure_resource()->get();
        result.width = max(result.width, size.width);
        result.height = max(result.height, size.height);
    }

    return result;
}

void CompositeWidget::add_child(std::shared_ptr<WidgetBase> widget) { 
    add_to_collection(ChildrenProperty, widget);
    widget->render_content()->bind(render_content());
    render_content()->bind(widget->render_content());
}

void CompositeWidget::remove_child(std::shared_ptr<WidgetBase> widget) { 
    remove_from_collection(ChildrenProperty, widget);
    widget->render_content()->unbind(render_content());
    render_content()->unbind(widget->render_content());
}

void CompositeWidget::layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) {
    WidgetBase::layout(constraints, layout_bounds, render_bounds);

    for (auto& child : m_children) {
        child->set_constraints(render_bounds);
        auto& widget_layout_bounds = child->layout_bounds_resource()->get();
        auto& widget_render_bounds = child->render_bounds_resource()->get();

        layout_bounds.left = min(layout_bounds.left, widget_layout_bounds.left);
        layout_bounds.top = min(layout_bounds.top, widget_layout_bounds.top);
        layout_bounds.right = max(layout_bounds.right, widget_layout_bounds.right);
        layout_bounds.bottom = max(layout_bounds.bottom, widget_layout_bounds.bottom);

        render_bounds.left = min(render_bounds.left, widget_render_bounds.left);
        render_bounds.top = min(render_bounds.top, widget_render_bounds.top);
        render_bounds.right = max(render_bounds.right, widget_render_bounds.right);
        render_bounds.bottom = max(render_bounds.bottom, widget_render_bounds.bottom);
    }
}

bool CompositeWidget::handle_pointer_hover(D2D1_POINT_2F point)
{
    for (auto& child : m_children)
    {
        if (child->hit_test(point))
        {
            if (child->handle_pointer_hover(point)) {
                return true;
            }
        }
    }

    return false;
}

bool CompositeWidget::handle_pointer_press(D2D1_POINT_2F point)
{
    for (auto& child : m_children)
    {
        if (child->hit_test(point))
        {
            if (child->handle_pointer_press(point)) {
                return true;
            }
        }
    }

    return false;
}

bool CompositeWidget::handle_pointer_release(D2D1_POINT_2F point)
{
    for (auto& child : m_children)
    {
        if (child->hit_test(point))
        {
            if (child->handle_pointer_release(point)) {
                return true;
            }
        }
    }

    return false;
}
