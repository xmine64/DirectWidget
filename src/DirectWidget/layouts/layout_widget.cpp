// base_layout_widget.cpp: BaseLayoutWidget implementation

#include <memory>
#include <type_traits>

#include "../core/foundation.hpp"
#include "../core/element_base.hpp"
#include "../core/widget.hpp"
#include "../core/dependency.hpp"
#include "../core/property.hpp"
#include "layout_widget.hpp"

using namespace DirectWidget;
using namespace Layouts;

class LayoutWidgetBase::ChildrenCollectionListener : public DependencyListenerBase {
    void on_dependency_updated(const ElementBase* owner, const NotificationArgument& arg) override {
        const auto& carg = static_cast<const CollectionUpdateNotificationArgument<widget_ptr>&>(arg);
        auto& layout = *const_cast<LayoutWidgetBase*>(static_cast<const LayoutWidgetBase*>(owner)); // FIXME:

        if (arg.notification_type() == NotificationType::ElementAdded) {
            auto node = std::make_unique<LAYOUT_NODE>();
            node->widget = carg.value();
            layout.m_nodes.push_back(std::move(node));
        }
        else if (arg.notification_type() == NotificationType::ElementRemoved) {
            layout.m_nodes.erase(std::remove_if(layout.m_nodes.begin(), layout.m_nodes.end(), [carg](const std::unique_ptr<LAYOUT_NODE>& node) {
                return node->widget == carg.value();
                }), layout.m_nodes.end());
        }

        // TODO: invalidate the layout
    }
};

collection_property_ptr<std::shared_ptr<WidgetBase>> LayoutWidgetBase::ChildrenProperty = []() {
    auto result = make_collection<std::shared_ptr<WidgetBase>>();
    result->add_listener(std::make_shared<ChildrenCollectionListener>());
    return result;
    }();

void LayoutWidgetBase::add_child(std::shared_ptr<WidgetBase> widget) { 
    add_to_collection(ChildrenProperty, widget);
    register_child(widget);
}

void LayoutWidgetBase::remove_child(std::shared_ptr<WidgetBase> widget) { 
    remove_from_collection(ChildrenProperty, widget);
    detach_child(widget);
}

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
