// base_layout_widget.cpp: BaseLayoutWidget implementation

#include <memory>
#include <type_traits>

#include "../core/widget.hpp"
#include "layout_widget.hpp"

using namespace DirectWidget;
using namespace Layouts;

void LayoutWidgetBase::on_children_changed(PropertyOwnerBase* sender, ObservableCollectionProperty<std::shared_ptr<WidgetBase>>* property, const std::shared_ptr<WidgetBase>& value, bool added)
{
    auto& layout = *static_cast<LayoutWidgetBase*>(sender);

    if (added) {
        auto node = std::make_unique<LAYOUT_NODE>();
        node->widget = value;
        layout.m_nodes.push_back(std::move(node));
    }
    else {
        layout.m_nodes.erase(std::remove_if(layout.m_nodes.begin(), layout.m_nodes.end(), [&value](const std::unique_ptr<LAYOUT_NODE>& node) {
            return node->widget == value;
            }), layout.m_nodes.end());
    }

    // TODO: invalidate the layout
}

collection_property_ptr<std::shared_ptr<WidgetBase>> LayoutWidgetBase::ChildrenProperty = make_collection<std::shared_ptr<WidgetBase>>(LayoutWidgetBase::on_children_changed);

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
