// text_widget.cpp: TextWidget implementation

#include <cstring>

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/app.hpp"
#include "../core/widget.hpp"
#include "text_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

property_ptr<PCWSTR> TextWidget::TextProperty = make_property<PCWSTR>(L"Text");
property_ptr<PCWSTR> TextWidget::FontFamilyProperty = make_property<PCWSTR>(L"Segoe UI");
property_ptr<float> TextWidget::FontSizeProperty = make_property(12.0f);
property_ptr<D2D1::ColorF> TextWidget::ColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::Black);
property_ptr<DWRITE_FONT_WEIGHT> TextWidget::FontWeightProperty = make_property(DWRITE_FONT_WEIGHT_NORMAL);
property_ptr<DWRITE_TEXT_ALIGNMENT> TextWidget::TextAlignmentProperty = make_property(DWRITE_TEXT_ALIGNMENT_LEADING);
property_ptr<DWRITE_PARAGRAPH_ALIGNMENT> TextWidget::ParagraphAlignmentProperty = make_property(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

SIZE_F DirectWidget::Widgets::TextWidget::measure(const SIZE_F& available_size) const
{
    m_text_layout->get()->SetMaxWidth(available_size.width);
    m_text_layout->get()->SetMaxHeight(available_size.height);

    DWRITE_TEXT_METRICS text_metrics;
    auto hr = m_text_layout->get()->GetMetrics(&text_metrics);
    if (FAILED(hr)) {
        _com_error err{ hr };
        OutputDebugString(L"GetMetrics failed:");
        OutputDebugString(err.ErrorMessage());

        return { 0, 0 };
    }

    return { text_metrics.width, text_metrics.height };
}

void DirectWidget::Widgets::TextWidget::on_layout_finalized(const BOUNDS_F& render_bounds)
{
    if (m_text_layout == nullptr)
        return;

    m_text_layout->get()->SetMaxWidth(render_bounds.right - render_bounds.left);
    m_text_layout->get()->SetMaxHeight(render_bounds.bottom - render_bounds.top);
}

void TextWidget::on_render() const
{
    if (m_text_layout == nullptr)
        return;

    render_target()->DrawTextLayout(
        D2D1::Point2F(render_bounds().left, render_bounds().top),
        m_text_layout->get(),
        m_text_fill->get(),
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}
