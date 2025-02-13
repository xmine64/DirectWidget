// base_layout_widget.cpp: BaseLayoutWidget implementation

#include "../core/widget.hpp"
#include "layout_widget.hpp"

using namespace DirectWidget;
using namespace Layouts;

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
