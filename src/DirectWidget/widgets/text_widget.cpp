// text_widget.cpp: TextWidget implementation

#include <cstring>

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"
#include "text_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

const LogContext TextWidget::Logger{ NAMEOF(TextWidget) };

property_ptr<PCWSTR> TextWidget::TextProperty = make_property<PCWSTR>(L"Text");
property_ptr<PCWSTR> TextWidget::FontFamilyProperty = make_property<PCWSTR>(L"Segoe UI");
property_ptr<float> TextWidget::FontSizeProperty = make_property(12.0f);
property_ptr<D2D1::ColorF> TextWidget::ColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::Black);
property_ptr<DWRITE_FONT_WEIGHT> TextWidget::FontWeightProperty = make_property(DWRITE_FONT_WEIGHT_NORMAL);
property_ptr<DWRITE_TEXT_ALIGNMENT> TextWidget::TextAlignmentProperty = make_property(DWRITE_TEXT_ALIGNMENT_LEADING);
property_ptr<DWRITE_PARAGRAPH_ALIGNMENT> TextWidget::ParagraphAlignmentProperty = make_property(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

TextWidget::TextWidget() {
    register_property(TextProperty, m_text);
    register_property(FontFamilyProperty, m_font_family);
    register_property(FontSizeProperty, m_size);
    register_property(ColorProperty, m_color);
    register_property(FontWeightProperty, m_weight);
    register_property(TextAlignmentProperty, m_alignment);
    register_property(ParagraphAlignmentProperty, m_vertical_alignment);

    m_text_fill = make_resource<ID2D1Brush>([this]() {
        com_ptr<ID2D1SolidColorBrush> brush;
        auto hr = render_target()->CreateSolidColorBrush(m_color, &brush);
        Logger.at(NAMEOF(m_text_fill)).fatal_exit(hr);
        return brush;
        });
    m_text_fill->bind(RenderTargetProperty);
    m_text_fill->bind(ColorProperty);
    render_content()->bind(m_text_fill);

    m_text_format = make_resource<IDWriteTextFormat>([this]() {
        auto& dwrite = Application::instance()->dwrite();

        com_ptr<IDWriteTextFormat> text_format;
        auto hr = dwrite->CreateTextFormat(
            m_font_family,
            NULL,
            m_weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            m_size,
            L"en-us",
            &text_format);
        Logger.at(NAMEOF(m_text_format)).fatal_exit(hr);

        hr = text_format->SetTextAlignment(m_alignment);
        Logger.at(NAMEOF(m_text_format)).fatal_exit(hr);

        text_format->SetParagraphAlignment(m_vertical_alignment);
        Logger.at(NAMEOF(m_text_format)).fatal_exit(hr);

        return text_format;
        });
    m_text_format->bind(FontFamilyProperty);
    m_text_format->bind(FontWeightProperty);
    m_text_format->bind(FontSizeProperty);
    m_text_format->bind(TextAlignmentProperty);
    m_text_format->bind(ParagraphAlignmentProperty);
    render_content()->bind(m_text_format);

    m_text_layout = make_resource<IDWriteTextLayout>([this]() {
        auto& dwrite = Application::instance()->dwrite();

        com_ptr<IDWriteTextLayout> text_layout;
        auto hr = dwrite->CreateTextLayout(
            m_text,
            static_cast<UINT32>(wcslen(m_text)),
            m_text_format->get(),
            render_bounds().right - render_bounds().left,
            render_bounds().bottom - render_bounds().top,
            &text_layout);
        Logger.at(NAMEOF(m_text_layout)).fatal_exit(hr);

        return text_layout;
        });
    m_text_layout->bind(TextProperty);
    m_text_layout->bind(m_text_format);
    add_listener(std::make_shared<TextLayoutUpdater>());
    render_content()->bind(m_text_layout);
}

SIZE_F TextWidget::measure(const SIZE_F& available_size) const
{
    m_text_layout->get()->SetMaxWidth(available_size.width);
    m_text_layout->get()->SetMaxHeight(available_size.height);

    DWRITE_TEXT_METRICS text_metrics;
    auto hr = m_text_layout->get()->GetMetrics(&text_metrics);
    if (FAILED(hr)) {
        Logger.at(NAMEOF(measure)).log_error(hr);

        return { 0, 0 };
    }

    return { text_metrics.width, text_metrics.height };
}

void TextWidget::render(const RenderContext& context) const
{
    context.render_target()->DrawTextLayout(
        D2D1::Point2F(render_bounds().left, render_bounds().top),
        m_text_layout->get(),
        m_text_fill->get(),
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}

void TextWidget::TextLayoutUpdater::on_property_changed(sender_ptr sender, property_base_ptr property)
{
    auto text_widget = static_cast<TextWidget*>(sender);

    if (!text_widget->m_text_layout->is_valid()) {
        return;
    }

    if (property == RenderBoundsProperty) {

        auto hr = text_widget->m_text_layout->get()->SetMaxWidth(text_widget->render_bounds().right - text_widget->render_bounds().left);
        Logger.at(NAMEOF(TextWidget::TextLayoutUpdater::on_property_changed)).log_error(hr);

        text_widget->m_text_layout->get()->SetMaxHeight(text_widget->render_bounds().bottom - text_widget->render_bounds().top);
        Logger.at(NAMEOF(TextWidget::TextLayoutUpdater::on_property_changed)).log_error(hr);
    }
}
