// base_layout_widget.cpp: BaseLayoutWidget implementation

#include <memory>
#include <type_traits>

#include "../core/widget.hpp"
#include "layout_widget.hpp"

using namespace DirectWidget;
using namespace Layouts;

collection_property_ptr<std::shared_ptr<WidgetBase>> LayoutWidgetBase::ChildrenProperty = make_collection<std::shared_ptr<WidgetBase>>();

void LayoutWidgetBase::create_resources()
{
    for (auto& node : m_nodes)
    {
        node->widget->create_resources();
    }
}

void LayoutWidgetBase::discard_resources()
{
    for (auto& node : m_nodes)
    {
        node->widget->discard_resources();
    }
}

bool LayoutWidgetBase::handle_pointer_hover(D2D1_POINT_2F point)
{
    for (auto& node : m_nodes)
    {
        if (node->widget->hit_test(point))
        {
            if (node->widget->handle_pointer_hover(point)) {
                return true;
            }
        }
    }

    return false;
}

bool LayoutWidgetBase::handle_pointer_press(D2D1_POINT_2F point)
{
    for (auto& node : m_nodes)
    {
        if (node->widget->hit_test(point))
        {
            if (node->widget->handle_pointer_press(point)) {
                return true;
            }
        }
    }

    return false;
}

bool LayoutWidgetBase::handle_pointer_release(D2D1_POINT_2F point)
{
    for (auto& node : m_nodes)
    {
        if (node->widget->hit_test(point))
        {
            if (node->widget->handle_pointer_release(point)) {
                return true;
            }
        }
    }

    return false;
}

void LayoutWidgetBase::LayoutWidgetChildrenListener::register_listener() {
    static bool is_registered = false;

    if (is_registered) return;
    ChildrenProperty->add_listener(std::move(std::make_shared<LayoutWidgetChildrenListener>()));
    is_registered = true;
}

void LayoutWidgetBase::LayoutWidgetChildrenListener::on_collection_changed(sender_ptr sender, property_token property, const std::shared_ptr<WidgetBase>& widget, bool add_or_remove) {
    auto& layout = *static_cast<LayoutWidgetBase*>(sender);

    if (add_or_remove) {
        auto node = std::make_unique<LAYOUT_NODE>();
        node->widget = widget;
        layout.m_nodes.push_back(std::move(node));
    }
    else {
        layout.m_nodes.erase(std::remove_if(layout.m_nodes.begin(), layout.m_nodes.end(), [&widget](const std::unique_ptr<LAYOUT_NODE>& node) {
            return node->widget == widget;
            }), layout.m_nodes.end());
    }

    // TODO: invalidate the layout
}
