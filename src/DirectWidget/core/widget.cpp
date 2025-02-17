// base_widget.cpp: BaseWidget implementation

#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <comdef.h>

#include "foundation.hpp"
#include "app.hpp"
#include "widget.hpp"
#include "property.hpp"

using namespace DirectWidget;

const LogContext WidgetBase::Logger{ NAMEOF(WidgetBase) };
const LogContext RenderContext::Logger{ NAMEOF(RenderContext) };

property_ptr<SIZE_F> WidgetBase::SizeProperty = make_property(SIZE_F{ 0, 0 });
property_ptr<BOUNDS_F> WidgetBase::MarginProperty = make_property(BOUNDS_F{ 0, 0, 0, 0 });
property_ptr<WIDGET_ALIGNMENT> WidgetBase::VerticalAlignmentProperty = make_property(WIDGET_ALIGNMENT_CENTER);
property_ptr<WIDGET_ALIGNMENT> WidgetBase::HorizontalAlignmentProperty = make_property(WIDGET_ALIGNMENT_CENTER);

property_base_ptr WidgetBase::RenderTargetProperty = std::make_shared<PropertyBase>();
property_base_ptr WidgetBase::RenderBoundsProperty = std::make_shared<PropertyBase>();

void WidgetBase::layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) const
{
    // Measure space required by the widget

    auto available_size = SIZE_F{
        constraints.right - constraints.left - m_margin.left - m_margin.right,
        constraints.bottom - constraints.top - m_margin.top - m_margin.bottom
    };

    if (m_size.width > 0) {
        available_size.width = min(available_size.width, m_size.width);
    }

    if (m_size.height > 0) {
        available_size.height = min(available_size.height, m_size.height);
    }

    auto size = measure(available_size);

    size.width = min(size.width, available_size.width);
    size.height = min(size.height, available_size.height);

    // Align widget in the given bounds

    switch (m_horizontal_alignment) {
    case WIDGET_ALIGNMENT_START:
        render_bounds.left = constraints.left + m_margin.left;
        render_bounds.right = render_bounds.left + size.width;
        break;

    case WIDGET_ALIGNMENT_CENTER:
    {
        auto free_width = (constraints.right - constraints.left) - (m_margin.right + m_margin.left) - size.width;
        render_bounds.left = constraints.left + m_margin.left + max(0.0f, free_width / 2);
        render_bounds.right = render_bounds.left + size.width;
    }
    break;

    case WIDGET_ALIGNMENT_STRETCH:
        render_bounds.left = constraints.left + m_margin.left;
        render_bounds.right = constraints.right - m_margin.right;
        break;

    case WIDGET_ALIGNMENT_END:
        render_bounds.right = constraints.right - m_margin.right;
        render_bounds.left = render_bounds.right - size.width;
        break;
    }

    switch (m_vertical_alignment) {
    case WIDGET_ALIGNMENT_START:
        render_bounds.top = constraints.top + m_margin.top;
        render_bounds.bottom = render_bounds.top + size.height;
        break;

    case WIDGET_ALIGNMENT_CENTER:
    {
        auto free_height = (constraints.bottom - constraints.top) - (m_margin.top + m_margin.bottom) - size.height;
        render_bounds.top = constraints.top + m_margin.top + max(0.0f, free_height / 2);
        render_bounds.bottom = render_bounds.top + size.height;
    }
    break;

    case WIDGET_ALIGNMENT_STRETCH:
        render_bounds.top = constraints.top + m_margin.top;
        render_bounds.bottom = constraints.bottom - m_margin.bottom;
        break;

    case WIDGET_ALIGNMENT_END:
        render_bounds.bottom = constraints.bottom - m_margin.bottom;
        render_bounds.top = render_bounds.bottom - size.height;
        break;
    }

    // Asign layout bounds

    layout_bounds.left = render_bounds.left - m_margin.left;
    layout_bounds.top = render_bounds.top - m_margin.top;
    layout_bounds.right = render_bounds.right + m_margin.right;
    layout_bounds.bottom = render_bounds.bottom + m_margin.bottom;
}

void WidgetBase::finalize_layout(const BOUNDS_F& render_bounds)
{
    m_layout.render_bounds = render_bounds;

    auto& d2d = DirectWidget::Application::instance()->d2d();
    auto hr = d2d->CreateRectangleGeometry(D2D1::RectF(render_bounds.left, render_bounds.top, render_bounds.right, render_bounds.bottom), reinterpret_cast<ID2D1RectangleGeometry**>(&m_layout.geometry));
    Logger.at(NAMEOF(finalize_layout)).at(NAMEOF(d2d->CreateRectangleGeometry)).fatal_exit(hr);

    notify_change(RenderBoundsProperty);
}

void WidgetBase::render_debug_layout(const com_ptr<ID2D1RenderTarget>& render_target) const
{
    if (m_layout.geometry == nullptr)
        return;

    com_ptr<ID2D1SolidColorBrush> bounds_brush;
    auto hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &bounds_brush);
    Logger.at(NAMEOF(render_debug_layout)).at(NAMEOF(render_target->CreateSolidColorBrush)).fatal_exit(hr);

    com_ptr<ID2D1SolidColorBrush> layout_brush;
    hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &layout_brush);
    Logger.at(NAMEOF(render_debug_layout)).at(NAMEOF(render_target->CreateSolidColorBrush)).fatal_exit(hr);

    auto render_bounds = D2D1::RectF(
        m_layout.render_bounds.left,
        m_layout.render_bounds.top,
        m_layout.render_bounds.right,
        m_layout.render_bounds.bottom);
    auto layout_bounds = D2D1::RectF(
        render_bounds.left - m_margin.left,
        render_bounds.top - m_margin.top,
        render_bounds.right + m_margin.right,
        render_bounds.bottom + m_margin.bottom);

    render_target->DrawRectangle(render_bounds, bounds_brush);
    render_target->DrawRectangle(layout_bounds, layout_brush);

    for_each_child([render_target](WidgetBase* widget) {
        widget->render_debug_layout(render_target);
        });

    hr = render_target->Flush();
    Logger.at(NAMEOF(render_debug_layout)).at(NAMEOF(render_target->Flush)).log_error(hr);
}

void WidgetBase::attach_render_target(const com_ptr<ID2D1RenderTarget>& render_target)
{
    m_render_target = render_target;
    for_each_child([render_target](WidgetBase* widget) {
        widget->attach_render_target(render_target);
        });

    notify_change(RenderTargetProperty);
}

void WidgetBase::detach_render_target()
{
    discard_resources();
    m_render_target = nullptr;
    for_each_child([](WidgetBase* widget) {
        widget->detach_render_target();
        });

    notify_change(RenderTargetProperty);
}

bool WidgetBase::hit_test(D2D1_POINT_2F point) const {
    if (m_layout.geometry == nullptr)
        return false;

    BOOL result = false;
    auto hr = m_layout.geometry->FillContainsPoint(point, D2D1::Matrix3x2F::Identity(), &result);
    Logger.at(NAMEOF(hit_test)).at(NAMEOF(m_layout.geometry->FillContainsPoint)).log_error(hr);

    return result;
}

void WidgetBase::RenderContentResource::initialize()
{
    if (is_valid()) return;

    RenderContext context{ m_widget->m_render_target, m_widget->m_layout.render_bounds };
    initialize_with_context(context);

    if (Application::instance()->is_debug()) {
        m_widget->render_debug_layout(context.render_target());
    }

    // TODO:
    // if (hr == D2DERR_RECREATE_TARGET)
}

void WidgetBase::RenderContentResource::initialize_with_context(const RenderContext& render_context)
{
    if (is_valid()) return;

    m_widget->render(render_context);
    auto hr = render_context.render_target()->Flush();
    Logger.at(NAMEOF(WidgetBase::RenderContentResource::initialize_with_context)).at(NAMEOF(ID2D1RenderTarget::Flush)).log_error(hr);

    m_widget->for_each_child([&render_context](WidgetBase* child) {
        auto child_render_context = render_context.render_child(child->render_bounds());
        auto child_render_content = static_pointer_cast<RenderContentResource>(child->render_content());
        child_render_content->initialize_with_context(child_render_context);
        });

    mark_valid();
}
