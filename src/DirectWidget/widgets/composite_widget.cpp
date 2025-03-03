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

void CompositeWidget::add_child(std::shared_ptr<WidgetBase> widget) {
    add_to_collection(ChildrenProperty, widget);
    register_child(widget);
}

void CompositeWidget::remove_child(std::shared_ptr<WidgetBase> widget) {
    remove_from_collection(ChildrenProperty, widget);
    detach_child(widget);
}

SIZE_F CompositeWidget::measure(const SIZE_F& available_size) const
{
    auto result = WidgetBase::measure(available_size);

    auto& children = ChildrenProperty->get_values(this);
    for (auto& child : children) {
        child->set_maximum_size(available_size);
        auto& size = MeasureResource->get_or_initialize_resource(child.get());
        result.width = max(result.width, size.width);
        result.height = max(result.height, size.height);
    }

    return result;
}

void CompositeWidget::layout(LayoutContext& context) const {
    WidgetBase::layout(context);

    auto& children = ChildrenProperty->get_values(this);
    widget_ptr previous_child;
    for (auto& child : children) {
        context.layout_child(child, context.render_bounds(), previous_child);
        previous_child = child;
    }
}

bool CompositeWidget::handle_pointer_hover(D2D1_POINT_2F point)
{
    auto& children = ChildrenProperty->get_values(this);
    for (auto& child : children)
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
    auto& children = ChildrenProperty->get_values(this);
    for (auto& child : children)
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
    auto& children = ChildrenProperty->get_values(this);
    for (auto& child : children)
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
