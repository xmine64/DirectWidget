// button_widget.cpp: ButtonWidget implementation

#include <memory>

#include <Windows.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"
#include "button_widget.hpp"
#include "text_widget.hpp"
#include "box_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

property_ptr<PCWSTR> ButtonWidget::TextProperty = make_property<PCWSTR>(L"Button");
property_ptr<BOUNDS_F> ButtonWidget::PaddingProperty = make_property(BOUNDS_F{ 4, 4, 4, 4 });
property_ptr<D2D1::ColorF> ButtonWidget::ForegroundColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::Black);
property_ptr<D2D1::ColorF> ButtonWidget::StrokeColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::Black);
property_ptr<D2D1::ColorF> ButtonWidget::BackgroundColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::LightGray);
property_ptr<D2D1::ColorF> ButtonWidget::HoverColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::DimGray);
property_ptr<D2D1::ColorF> ButtonWidget::PressedColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::DarkGray);

ButtonWidget::ButtonWidget()
{
    register_property(TextProperty, m_text);
    register_property(PaddingProperty, m_padding);
    register_property(ForegroundColorProperty, m_foreground_color);
    register_property(StrokeColorProperty, m_stroke_color);
    register_property(BackgroundColorProperty, m_background_color);
    register_property(HoverColorProperty, m_hover_color);
    register_property(PressedColorProperty, m_pressed_color);

    m_box_widget = std::make_shared<BoxWidget>();
    m_box_widget->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
    m_box_widget->set_vertical_alignment(WIDGET_ALIGNMENT_STRETCH);
    add_child(m_box_widget);

    m_text_widget = std::make_shared<TextWidget>();
    m_text_widget->set_text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_text_widget->set_font_size(14.0f);
    add_child(m_text_widget);
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

    m_text_widget->set_maximum_size(text_available_size);
    auto text_measures = m_text_widget->measure_resource()->get();

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
