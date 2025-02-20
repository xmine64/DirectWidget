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

void CompositeWidget::layout(LayoutContext& context) const {
    WidgetBase::layout(context);

    widget_ptr previous_child;
    for (auto& child : m_children) {
        context.layout_child(child, context.render_bounds(), previous_child);
        previous_child = child;
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
