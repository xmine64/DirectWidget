// box_widget.cpp: Box widget implementation

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "../core/property.hpp"
#include "box_widget.hpp"

#define ASSERT(s) {auto hr = s; if (FAILED(s)) { _com_error err{ hr }; OutputDebugStringA(#s" failed:"); OutputDebugString(err.ErrorMessage()); return; }}

using namespace DirectWidget;
using namespace Widgets;

property_ptr<D2D1::ColorF> BoxWidget::BackgroundColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::White);
property_ptr<D2D1::ColorF> BoxWidget::StrokeColorProperty = make_property<D2D1::ColorF>(D2D1::ColorF::Black);

void BoxWidget::create_resources()
{
    ASSERT(render_target()->CreateSolidColorBrush(m_background_color, reinterpret_cast<ID2D1SolidColorBrush**>(&m_background_brush)));

    ASSERT(render_target()->CreateSolidColorBrush(m_stroke_color, reinterpret_cast<ID2D1SolidColorBrush**>(&m_stroke_brush)));
}

void BoxWidget::on_render() const
{
    auto box = D2D1::RectF(
        render_bounds().left,
        render_bounds().top,
        render_bounds().right,
        render_bounds().bottom
    );
    render_target()->FillRectangle(box, m_background_brush);
    render_target()->DrawRectangle(box, m_stroke_brush);
}
