// box_widget.cpp: Box widget implementation

#include <memory>

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "../core/foundation.hpp"
#include "../core/element_base.hpp"
#include "../core/property.hpp"
#include "../core/resource.hpp"
#include "../core/interop.hpp"
#include "../core/widget.hpp"
#include "box_widget.hpp"

using namespace DirectWidget;
using namespace Widgets;

const LogContext BoxWidget::m_log{ NAMEOF(BoxWidget) };

// Properties

property_ptr<D2D1_COLOR_F> BoxWidget::BackgroundColorProperty = make_property<D2D1_COLOR_F>(D2D1::ColorF(D2D1::ColorF::White));
property_ptr<D2D1_COLOR_F> BoxWidget::StrokeColorProperty = make_property<D2D1_COLOR_F>(D2D1::ColorF(D2D1::ColorF::Black));
property_ptr<float> BoxWidget::StrokeWidthProperty = make_property<float>(1.0f);

// Resources

Interop::com_resource_ptr<ID2D1SolidColorBrush> BoxWidget::FillBrushResource = 
    std::make_shared<Interop::SolidColorBrushResource>(BackgroundColorProperty);

Interop::com_resource_ptr<ID2D1SolidColorBrush> BoxWidget::StrokeBrushResource = 
    std::make_shared<Interop::SolidColorBrushResource>(StrokeColorProperty);

BoxWidget::BoxWidget() {
    register_dependency(BackgroundColorProperty);
    register_dependency(StrokeColorProperty);
    register_dependency(StrokeWidthProperty);

    register_dependency(FillBrushResource);
    register_dependency(StrokeBrushResource);
}

void BoxWidget::render(const RenderContext& context) const
{
    auto box = D2D1::RectF(
        context.render_bounds().left,
        context.render_bounds().top,
        context.render_bounds().right,
        context.render_bounds().bottom
    );
    auto& fill_brush = FillBrushResource->get_or_initialize_resource(this);
    context.render_target()->FillRectangle(box, fill_brush);

    auto width = stroke_width();
    auto& stroke_brush = StrokeBrushResource->get_or_initialize_resource(this);
    if (width > 0.0f) {
        context.render_target()->DrawRectangle(box, stroke_brush, width);
    }
}
