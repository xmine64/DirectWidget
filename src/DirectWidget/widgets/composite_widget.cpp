// composite_widget.cpp: Composite widget implementation

#include <Windows.h>

#include "../core/foundation.hpp"
#include "../core/widget.hpp"
#include "composite_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

SIZE_F CompositeWidget::measure(const SIZE_F& available_size) const
{
    auto result = WidgetBase::measure(available_size);

    for (auto& child : m_children) {
        auto size = child->measure(available_size);
        result.width = max(result.width, size.width);
        result.height = max(result.height, size.height);
    }

    return result;
}

void CompositeWidget::layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) const {
    WidgetBase::layout(constraints, layout_bounds, render_bounds);

    for (auto& child : m_children) {
        auto widget_layout_bounds = layout_bounds;
        auto widget_render_bounds = render_bounds;
        child->layout(render_bounds, widget_layout_bounds, widget_render_bounds);
        child->finalize_layout(widget_render_bounds);

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
