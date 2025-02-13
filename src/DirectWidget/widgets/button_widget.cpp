// button_widget.cpp: ButtonWidget implementation

#include <memory>

#include "../core/foundation.hpp"
#include "../core/widget.hpp"
#include "button_widget.hpp"
#include "text_widget.hpp"
#include "box_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

ButtonWidget::ButtonWidget()
{
    m_box_widget = std::make_shared<BoxWidget>();
    m_box_widget->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
    m_box_widget->set_vertical_alignment(WIDGET_ALIGNMENT_STRETCH);
    add_widget(m_box_widget);

    m_text_widget = std::make_shared<TextWidget>();
    m_text_widget->set_text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_text_widget->set_font_size(14.0f);
    add_widget(m_text_widget);
}

void ButtonWidget::create_resources() {
    m_text_widget->set_text(m_text);
    m_text_widget->set_horizontal_alignment(WIDGET_ALIGNMENT_CENTER);
    m_text_widget->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
    m_text_widget->set_font_size(14.0f);
    m_text_widget->set_color(m_foreground_color);
    m_text_widget->set_margin(m_padding);

    m_box_widget->set_background_color(m_background_color);
    m_box_widget->set_stroke_color(m_stroke_color);

    WidgetBase::create_resources();
}

SIZE_F ButtonWidget::measure(const SIZE_F& available_size) const {
    if (m_text_widget == nullptr)
    {
        return { 0,0 };
    }

    auto text_available_size = SIZE_F{
        available_size.width - m_padding.left - m_padding.right,
        available_size.height - m_padding.top - m_padding.bottom
    };

    auto text_measures = m_text_widget->measure(text_available_size);

    return {
        text_measures.width + m_padding.left + m_padding.right,
        text_measures.height + m_padding.top + m_padding.bottom
    };
}

bool ButtonWidget::handle_pointer_hover(D2D1_POINT_2F point) {
    // TODO
    return false;
}

bool ButtonWidget::handle_pointer_press(D2D1_POINT_2F point) {
    // TODO
    return false;
}

bool ButtonWidget::handle_pointer_release(D2D1_POINT_2F point) {
    if (m_click_handler != nullptr) {
        m_click_handler();
        return true;
    }

    return false;
}
