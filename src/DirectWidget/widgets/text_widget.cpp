// text_widget.cpp: TextWidget implementation

#include <cstring>

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/foundation.hpp"
#include "../core/app.hpp"
#include "../core/widget.hpp"
#include "text_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

void TextWidget::create_resources()
{
    auto& dwrite = Application::instance()->dwrite();

    HRESULT hr;

    if (m_text_format == nullptr)
    {
        hr = dwrite->CreateTextFormat(
            m_font_family,
            NULL,
            m_weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            m_size,
            L"en-us",
            &m_text_format);

        if (FAILED(hr)) {
            _com_error err{ hr };
            OutputDebugString(L"CreateTextFormat failed:");
            OutputDebugString(err.ErrorMessage());
            return;
        }

        hr = m_text_format->SetTextAlignment(m_alignment);
        if (FAILED(hr)) {
            _com_error err{ hr };
            OutputDebugString(L"SetTextAlignment failed:");
            OutputDebugString(err.ErrorMessage());
            return;
        }

        hr = m_text_format->SetParagraphAlignment(m_vertical_alignment);
        if (FAILED(hr)) {
            _com_error err{ hr };
            OutputDebugString(L"SetParagraphAlignment failed:");
            OutputDebugString(err.ErrorMessage());
            return;
        }
    }

    if (m_brush == nullptr)
    {
        hr = render_target()->CreateSolidColorBrush(m_color, &m_brush);
        if (FAILED(hr)) {
            _com_error err{ hr };
            OutputDebugString(L"CreateSolidColorBrush failed:");
            OutputDebugString(err.ErrorMessage());
            return;
        }
    }

    if (m_text_layout == nullptr) {
        hr = dwrite->CreateTextLayout(
            m_text,
            wcslen(m_text),
            m_text_format,
            render_bounds().right - render_bounds().left,
            render_bounds().bottom - render_bounds().top,
            &m_text_layout);
        if (FAILED(hr)) {
            _com_error err{ hr };
            OutputDebugString(L"CreateSolidColorBrush failed:");
            OutputDebugString(err.ErrorMessage());
            return;
        }
    }
}

void TextWidget::discard_resources()
{
    m_text_layout = nullptr;
    m_text_format = nullptr;
    m_brush = nullptr;
}

SIZE_F DirectWidget::Widgets::TextWidget::measure(const SIZE_F& available_size) const
{
    if (m_text_format == nullptr) {
        return { 0, 0 };
    }

    m_text_layout->SetMaxWidth(available_size.width);
    m_text_layout->SetMaxHeight(available_size.height);

    DWRITE_TEXT_METRICS text_metrics;
    auto hr = m_text_layout->GetMetrics(&text_metrics);
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

    m_text_layout->SetMaxWidth(render_bounds.right - render_bounds.left);
    m_text_layout->SetMaxHeight(render_bounds.bottom - render_bounds.bottom);
}

void TextWidget::on_render() const
{
    if (m_text_layout == nullptr)
        return;

    render_target()->DrawTextLayout(
        D2D1::Point2F(render_bounds().left, render_bounds().top),
        m_text_layout,
        m_brush,
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}
