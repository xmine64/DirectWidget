// base_widget.cpp: BaseWidget implementation

#include <memory>

#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <comdef.h>

#include "foundation.hpp"
#include "app.hpp"
#include "widget.hpp"
#include "property.hpp"
#include "resource.hpp"

using namespace DirectWidget;

const LogContext WidgetBase::Logger{ NAMEOF(WidgetBase) };
const LogContext RenderContext::Logger{ NAMEOF(RenderContext) };

property_ptr<SIZE_F> WidgetBase::SizeProperty = make_property(SIZE_F{ 0, 0 });
property_ptr<BOUNDS_F> WidgetBase::MarginProperty = make_property(BOUNDS_F{ 0, 0, 0, 0 });
property_ptr<WIDGET_ALIGNMENT> WidgetBase::VerticalAlignmentProperty = make_property(WIDGET_ALIGNMENT_CENTER);
property_ptr<WIDGET_ALIGNMENT> WidgetBase::HorizontalAlignmentProperty = make_property(WIDGET_ALIGNMENT_CENTER);

property_ptr<SIZE_F> WidgetBase::MaxSizeProperty = make_property<SIZE_F>({ 0,0 });
property_ptr<BOUNDS_F> WidgetBase::ConstraintsProperty = make_property<BOUNDS_F>({ 0,0,0,0 });

property_base_ptr WidgetBase::RenderTargetProperty = std::make_shared<PropertyBase>();
property_base_ptr WidgetBase::RenderBoundsProperty = std::make_shared<PropertyBase>();

WidgetBase::WidgetBase() {
    register_property(SizeProperty, m_size);
    register_property(MarginProperty, m_margin);
    register_property(VerticalAlignmentProperty, m_vertical_alignment);
    register_property(HorizontalAlignmentProperty, m_horizontal_alignment);

    register_property(MaxSizeProperty, m_max_size);
    register_property(ConstraintsProperty, m_constraints);

    m_measure = make_resource<SIZE_F>([this]() {
        SIZE_F available_size{ maximum_size() };

        if (m_size.width > 0) {
            available_size.width = min(available_size.width, m_size.width);
        }

        if (m_size.height > 0) {
            available_size.height = min(available_size.height, m_size.height);
        }
        
        auto margin_width = m_margin.left + m_margin.right;
        auto margin_height = m_margin.top + m_margin.bottom;

        available_size.width -= margin_width;
        available_size.height -= margin_height;

        auto content_size = measure(available_size);
        
        return SIZE_F{
            content_size.width + margin_width,
            content_size.height + margin_height
        };
        });
    m_measure->bind(MaxSizeProperty);

    m_layout_bounds = make_resource<BOUNDS_F>([this]() {
        BOUNDS_F layout_bounds;
        BOUNDS_F render_bounds;
        layout(m_constraints, layout_bounds, render_bounds);
        finalize_layout(render_bounds);

        return layout_bounds;
        });
    m_layout_bounds->bind(ConstraintsProperty);
    m_layout_bounds->bind(m_measure);

    m_render_content = std::make_shared<RenderContentResource>(this);
    m_render_content->bind(RenderBoundsProperty);
}

void WidgetBase::layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds)
{
    // Measure space required by the widget

    auto maximum_size = SIZE_F{
        constraints.right - constraints.left,
        constraints.bottom - constraints.top
    };
    set_maximum_size(maximum_size);
    SIZE_F size{ m_measure->get() };

    size.width = min(size.width, maximum_size.width);
    size.height = min(size.height, maximum_size.height);

    // Align widget in the given bounds

    switch (m_horizontal_alignment) {
    case WIDGET_ALIGNMENT_START:
        layout_bounds.left = constraints.left;
        layout_bounds.right = layout_bounds.left + size.width;
        break;

    case WIDGET_ALIGNMENT_CENTER:
    {
        auto free_width = (constraints.right - constraints.left) - size.width;
        layout_bounds.left = constraints.left + max(0.0f, free_width / 2);
        layout_bounds.right = layout_bounds.left + size.width;
    }
    break;

    case WIDGET_ALIGNMENT_STRETCH:
        layout_bounds.left = constraints.left;
        layout_bounds.right = constraints.right;
        break;

    case WIDGET_ALIGNMENT_END:
        layout_bounds.right = constraints.right;
        layout_bounds.left = layout_bounds.right - size.width;
        break;
    }

    switch (m_vertical_alignment) {
    case WIDGET_ALIGNMENT_START:
        layout_bounds.top = constraints.top;
        layout_bounds.bottom = layout_bounds.top + size.height;
        break;

    case WIDGET_ALIGNMENT_CENTER:
    {
        auto free_height = (constraints.bottom - constraints.top) - size.height;
        layout_bounds.top = constraints.top + max(0.0f, free_height / 2);
        layout_bounds.bottom = layout_bounds.top + size.height;
    }
    break;

    case WIDGET_ALIGNMENT_STRETCH:
        layout_bounds.top = constraints.top;
        layout_bounds.bottom = constraints.bottom;
        break;

    case WIDGET_ALIGNMENT_END:
        layout_bounds.bottom = constraints.bottom;
        layout_bounds.top = layout_bounds.bottom - size.height;
        break;
    }

    // Asign render bounds

    render_bounds.left = layout_bounds.left + m_margin.left;
    render_bounds.top = layout_bounds.top + m_margin.top;
    render_bounds.right = layout_bounds.right - m_margin.right;
    render_bounds.bottom = layout_bounds.bottom - m_margin.bottom;
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

    m_widget->layout_bounds_resource()->initialize();
    RenderContext context{ m_widget->render_target(), m_widget->render_bounds()};
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
        child->layout_bounds_resource()->initialize();
        auto child_render_context = render_context.create_subcontext(child->render_bounds());
        auto child_render_content = static_pointer_cast<RenderContentResource>(child->render_content());
        child_render_content->initialize_with_context(child_render_context);
        });

    mark_valid();
}
