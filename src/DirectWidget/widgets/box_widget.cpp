// box_widget.cpp: Box widget implementation

#include <d2d1.h>
#include <d2d1helper.h>

#include "box_widget.hpp"

#define ASSERT(s) {auto hr = s; if (FAILED(s)) { _com_error err{ hr }; OutputDebugStringA(#s" failed:"); OutputDebugString(err.ErrorMessage()); return; }}

void DirectWidget::Widgets::BoxWidget::create_resources()
{
    ASSERT(render_target()->CreateSolidColorBrush(m_background_color, reinterpret_cast<ID2D1SolidColorBrush**>(&m_background_brush)));

    ASSERT(render_target()->CreateSolidColorBrush(m_stroke_color, reinterpret_cast<ID2D1SolidColorBrush**>(&m_stroke_brush)));
}

void DirectWidget::Widgets::BoxWidget::on_render() const
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
