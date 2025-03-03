// text_widget.cpp: TextWidget implementation

#include <memory>
#include <cstring>

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/foundation.hpp"
#include "../core/element_base.hpp"
#include "../core/property.hpp"
#include "../core/interop.hpp"
#include "../core/app.hpp"
#include "../core/widget.hpp"
#include "text_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

const LogContext TextWidget::Logger{ NAMEOF(TextWidget) };

class TextWidget::DWriteTextFormatResource : public Interop::ComResource<IDWriteTextFormat> {
protected:
    HRESULT initialize(const ElementBase* owner, Interop::com_ptr<IDWriteTextFormat>& resource) override {
        auto& dwrite = Application::instance()->dwrite();

        auto& font_family = TextWidget::FontFamilyProperty->get_value(owner);
        auto& font_weight = TextWidget::FontWeightProperty->get_value(owner);
        auto& font_size = TextWidget::FontSizeProperty->get_value(owner);
        auto& text_alignment = TextWidget::TextAlignmentProperty->get_value(owner);
        auto& paragraph_alignment = TextWidget::ParagraphAlignmentProperty->get_value(owner);

        auto hr = dwrite->CreateTextFormat(
            font_family,
            NULL,
            font_weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            font_size,
            L"en-us",
            &resource);
        if (FAILED(hr)) {
            return hr;
        }

        hr = resource->SetTextAlignment(text_alignment);
        if (FAILED(hr)) {
            return hr;
        }

        hr = resource->SetParagraphAlignment(paragraph_alignment);
        return hr;
    }
};

class TextWidget::DWriteTextLayoutResource : public Interop::ComResource<IDWriteTextLayout> {
protected:
    HRESULT initialize(const ElementBase* owner, Interop::com_ptr<IDWriteTextLayout>& resource) override {
        auto& dwrite = Application::instance()->dwrite();

        auto& render_bounds = WidgetBase::RenderBoundsResource->get_or_initialize_resource(owner);
        auto& text = TextWidget::TextProperty->get_value(owner);
        auto& text_format = TextWidget::TextFormatResource->get_or_initialize_resource(owner);

        return dwrite->CreateTextLayout(
            text,
            static_cast<UINT32>(wcslen(text)),
            text_format,
            render_bounds.right - render_bounds.left,
            render_bounds.bottom - render_bounds.top,
            &resource);
    };
};

// properties

property_ptr<PCWSTR> TextWidget::TextProperty = make_property<PCWSTR>(L"Text");
property_ptr<PCWSTR> TextWidget::FontFamilyProperty = make_property<PCWSTR>(L"Segoe UI");
property_ptr<float> TextWidget::FontSizeProperty = make_property(12.0f);
property_ptr<D2D1_COLOR_F> TextWidget::ColorProperty = make_property<D2D1_COLOR_F>(D2D1::ColorF(D2D1::ColorF::Black));
property_ptr<DWRITE_FONT_WEIGHT> TextWidget::FontWeightProperty = make_property(DWRITE_FONT_WEIGHT_NORMAL);
property_ptr<DWRITE_TEXT_ALIGNMENT> TextWidget::TextAlignmentProperty = make_property(DWRITE_TEXT_ALIGNMENT_LEADING);
property_ptr<DWRITE_PARAGRAPH_ALIGNMENT> TextWidget::ParagraphAlignmentProperty = make_property(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

// resources

Interop::com_resource_ptr<ID2D1SolidColorBrush> TextWidget::TextFillResource = std::make_shared<Interop::SolidColorBrushResource>(ColorProperty);
Interop::com_resource_ptr<IDWriteTextFormat> TextWidget::TextFormatResource = std::make_shared<DWriteTextFormatResource>();
Interop::com_resource_ptr<IDWriteTextLayout> TextWidget::TextLayoutResource = std::make_shared<DWriteTextLayoutResource>();

TextWidget::TextWidget() {
    register_dependency(TextProperty);
    register_dependency(FontFamilyProperty);
    register_dependency(FontSizeProperty);
    register_dependency(ColorProperty);
    register_dependency(FontWeightProperty);
    register_dependency(TextAlignmentProperty);
    register_dependency(ParagraphAlignmentProperty);

    register_dependency(TextFillResource);
    register_dependency(TextFormatResource);
    register_dependency(TextLayoutResource);
}

SIZE_F TextWidget::measure(const SIZE_F& available_size) const
{
    auto& dwrite = Application::instance()->dwrite();

    auto& text = TextProperty->get_value(this);

    Interop::com_ptr<IDWriteTextLayout> text_layout;
    auto hr = dwrite->CreateTextLayout(
        text,
        static_cast<UINT32>(wcslen(text)),
        TextWidget::TextFormatResource->get_or_initialize_resource(this),
        available_size.width,
        available_size.height,
        &text_layout);
    if (FAILED(hr)) {
        Logger.at(NAMEOF(measure)).log_error(hr);
        return { 0, 0 };
    }

    DWRITE_TEXT_METRICS text_metrics;
    hr = text_layout->GetMetrics(&text_metrics);
    if (FAILED(hr)) {
        Logger.at(NAMEOF(measure)).log_error(hr);
        return { 0, 0 };
    }

    return { text_metrics.widthIncludingTrailingWhitespace + 1.0f, text_metrics.height + 1.0f };
}

void TextWidget::render(const RenderContext& context) const
{
    context.render_target()->DrawTextLayout(
        D2D1::Point2F(context.render_bounds().left, context.render_bounds().top),
        TextLayoutResource->get_or_initialize_resource(this),
        TextFillResource->get_or_initialize_resource(this),
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}
