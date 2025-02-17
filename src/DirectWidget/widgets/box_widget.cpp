// box_widget.cpp: Box widget implementation

#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/resource.hpp"
#include "../core/widget.hpp"
#include "box_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

const LogContext BoxWidget::m_log{ NAMEOF(BoxWidget) };

property_ptr<D2D1::ColorF> BoxWidget::BackgroundColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::White);
property_ptr<D2D1::ColorF> BoxWidget::StrokeColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::Black);
property_ptr<float> BoxWidget::StrokeWidthProperty = make_property<float>(1.0f);

BoxWidget::BoxWidget() {
    register_property(BackgroundColorProperty, m_background_color);
    register_property(StrokeColorProperty, m_stroke_color);
    register_property(StrokeWidthProperty, m_stroke_width);

    render_content()->bind(StrokeWidthProperty);

    m_background_brush = make_resource<ID2D1Brush>([this]() {
        com_ptr<ID2D1SolidColorBrush> m_brush;
        auto hr = render_target()->CreateSolidColorBrush(m_background_color, &m_brush);
        m_log.at(NAMEOF(m_background_brush)).fatal_exit(hr);
        return m_brush;
        });
    m_background_brush->bind(RenderTargetProperty);
    m_background_brush->bind(BackgroundColorProperty);
    render_content()->bind(m_background_brush);

    m_stroke_brush = make_resource<ID2D1Brush>([this]() {
        com_ptr<ID2D1SolidColorBrush> m_brush;
        auto hr = render_target()->CreateSolidColorBrush(m_stroke_color, &m_brush);
        m_log.at(NAMEOF(m_stroke_brush)).fatal_exit(hr);
        return m_brush;
        });
    m_stroke_brush->bind(RenderTargetProperty);
    m_stroke_brush->bind(StrokeColorProperty);
    render_content()->bind(m_stroke_brush);
}

void BoxWidget::render(const RenderContext& context) const
{
    auto box = D2D1::RectF(
        render_bounds().left,
        render_bounds().top,
        render_bounds().right,
        render_bounds().bottom
    );
    context.render_target()->FillRectangle(box, m_background_brush->get());

    if (m_stroke_width > 0.0f) {
        context.render_target()->DrawRectangle(box, m_stroke_brush->get(), m_stroke_width);
    }
}
