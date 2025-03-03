// stack_layout.cpp: StackLayout implementation
// StackLayout is a widget that arranges its children in a horizontal or vertical stack.

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"
#include "stack_layout.hpp"

using namespace DirectWidget;
using namespace Layouts;

property_ptr<STACK_LAYOUT_ORIENTATION> StackLayout::OrientationProperty = make_property(STACK_LAYOUT_HORIZONTAL);

void StackLayout::layout(LayoutContext& context) const
{
    WidgetBase::layout(context);

    BOUNDS_F available_bounds{ context.render_bounds() };

    auto& orientation = get_property(OrientationProperty);

    for (auto& node : m_nodes) {
        auto node_constraints = BOUNDS_F{
            available_bounds.left,
            available_bounds.top,
            available_bounds.left + node->layout_size.width,
            available_bounds.top + node->layout_size.height
        };
        if (orientation == STACK_LAYOUT_HORIZONTAL) {
            node_constraints.bottom = available_bounds.bottom;
        }
        else {
            node_constraints.right = available_bounds.right;
        }
        context.layout_child(node->widget, node_constraints);
        
        if (orientation == STACK_LAYOUT_HORIZONTAL) {
            available_bounds.left += node->layout_size.width;
        }
        else {
            available_bounds.top += node->layout_size.height;
        }
    }
}

SIZE_F StackLayout::measure(const SIZE_F& available_size) const
{
    auto& orientation = get_property(OrientationProperty);

    auto flex_size = orientation == STACK_LAYOUT_HORIZONTAL
        ? available_size.width
        : available_size.height;
    auto flex_count = 0;

    // First pass: Measure non-stretch widgets and determine flex count
    for (auto& node : m_nodes) {
        if (orientation == STACK_LAYOUT_HORIZONTAL) {
            if (node->widget->horizontal_alignment() == WidgetAlignment::Stretch) {
                flex_count++;
            }
            else {
                node->widget->set_maximum_size(SIZE_F{ flex_size, available_size.height });
                node->measure = WidgetBase::MeasureResource->get_or_initialize_resource(node->widget.get());
                node->layout_size.width = node->measure.width;
                flex_size -= node->layout_size.width;
            }
        }
        else {
            if (node->widget->vertical_alignment() == WidgetAlignment::Stretch) {
                flex_count++;
            }
            else {
                node->widget->set_maximum_size(SIZE_F{ available_size.width, flex_size });
                node->measure = WidgetBase::MeasureResource->get_or_initialize_resource(node->widget.get());
                node->layout_size.height = node->measure.height;
                flex_size -= node->layout_size.height;
            }
        }
    }

    auto non_flex_size = (orientation == STACK_LAYOUT_HORIZONTAL
        ? available_size.width
        : available_size.height) - flex_size;

    if (flex_count > 0) {
        flex_size /= flex_count;
    }

    auto sum = SIZE_F{ 0, 0 };

    // Second pass: Assign sizes to stretch widgets and compute total size
    for (auto& node : m_nodes) {
        if (orientation == STACK_LAYOUT_HORIZONTAL) {
            if (node->widget->horizontal_alignment() == WidgetAlignment::Stretch) {
                node->widget->set_maximum_size(SIZE_F{ flex_size, available_size.height });
                node->measure = WidgetBase::MeasureResource->get_or_initialize_resource(node->widget.get());
                node->layout_size.width = flex_size;
            }
            else {
                node->widget->set_maximum_size(SIZE_F{ non_flex_size, available_size.height });
                node->measure = WidgetBase::MeasureResource->get_or_initialize_resource(node->widget.get());
                node->layout_size.width = node->measure.width;
            }

            node->layout_size.height = node->measure.height;

            sum.width += node->layout_size.width;
            sum.height = max(sum.height, node->layout_size.height);
        }
        else {
            if (node->widget->vertical_alignment() == WidgetAlignment::Stretch) {
                node->widget->set_maximum_size(SIZE_F{ available_size.width, flex_size });
                node->measure = WidgetBase::MeasureResource->get_or_initialize_resource(node->widget.get());
                node->layout_size.height = flex_size;
            }
            else {
                node->widget->set_maximum_size(SIZE_F{ available_size.width, non_flex_size });
                node->measure = WidgetBase::MeasureResource->get_or_initialize_resource(node->widget.get());
                node->layout_size.height = node->measure.height;
            }

            node->layout_size.width = node->measure.width;

            sum.width = max(sum.width, node->layout_size.width);
            sum.height += node->layout_size.height;
        }
    }

    return sum;
}
