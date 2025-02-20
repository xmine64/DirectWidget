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
    //m_measure->bind(MaxSizeProperty);
    //m_measure->bind(MarginProperty);

    m_layout = std::make_shared<LayoutResource>(this);
    m_layout->bind(m_measure);
    //m_layout->bind(ConstraintsProperty);
    //m_layout->bind(MarginProperty);

    m_render_bounds = make_resource<BOUNDS_F>([this]() {
        return m_layout->get().render_bounds();
        });
    m_render_bounds->bind(m_layout);

    m_render_geometry = make_resource<ID2D1Geometry>([this]() {
        auto& render_bounds = m_render_bounds->get();
        auto rect = D2D1::RectF(render_bounds.left, render_bounds.top, render_bounds.right, render_bounds.bottom);

        auto& d2d = DirectWidget::Application::instance()->d2d();
        com_ptr<ID2D1RectangleGeometry> result;
        auto hr = d2d->CreateRectangleGeometry(rect, &result);
        Logger.at(NAMEOF(m_render_geometry)).at(NAMEOF(ID2D1Factory::CreateRectangleGeometry)).fatal_exit(hr);
        return result;
        });
    m_render_geometry->bind(m_layout);

    m_render_content = std::make_shared<RenderContentResource>(this);
    m_render_content->bind(m_layout);
}

void WidgetBase::layout(LayoutContext& context) const
{
    // Align widget in the given bounds

    auto& size = context.measure();
    auto& constraints = context.constraints();
    auto& layout_bounds = context.layout_bounds();

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
}

void WidgetBase::render_debug_layout(const com_ptr<ID2D1RenderTarget>& render_target) const
{
    com_ptr<ID2D1SolidColorBrush> bounds_brush;
    auto hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &bounds_brush);
    Logger.at(NAMEOF(render_debug_layout)).at(NAMEOF(render_target->CreateSolidColorBrush)).fatal_exit(hr);

    com_ptr<ID2D1SolidColorBrush> layout_brush;
    hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &layout_brush);
    Logger.at(NAMEOF(render_debug_layout)).at(NAMEOF(render_target->CreateSolidColorBrush)).fatal_exit(hr);

    auto& layout_bounds = m_layout->get().layout_bounds();
    auto& render_bounds = m_render_bounds->get();

    auto layout_rect = D2D1::RectF(layout_bounds.left, layout_bounds.top, layout_bounds.right, layout_bounds.bottom);
    auto render_rect = D2D1::RectF(render_bounds.left, render_bounds.top, render_bounds.right, render_bounds.bottom);

    render_target->DrawRectangle(render_rect, bounds_brush);
    render_target->DrawRectangle(layout_rect, layout_brush);

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
    BOOL result = false;
    auto hr = m_render_geometry->get()->FillContainsPoint(point, D2D1::Matrix3x2F::Identity(), &result);
    Logger.at(NAMEOF(hit_test)).at(NAMEOF(ID2D1Geometry::FillContainsPoint)).log_error(hr);
    return result;
}

void WidgetBase::LayoutResource::initialize()
{
    if (is_valid()) return;

    m_context = LayoutContext(m_owner->m_measure->get(), m_owner->m_constraints, m_owner->m_margin, m_context.background_widget());
    m_owner->layout(m_context);
    mark_valid();
}

void WidgetBase::LayoutResource::initialize_with_context(const LayoutContext& context)
{
    if (is_valid()) return;

    m_context = context;
    m_owner->layout(m_context);
    mark_valid();
}

void LayoutContext::layout_child(const widget_ptr& child, const BOUNDS_F& constraints, const widget_ptr& background) const {
    child->set_constraints(constraints);
    auto child_layout = static_pointer_cast<WidgetBase::LayoutResource>(child->layout_resource());
    auto child_context = create_subcontext(child->measure_resource()->get(), child->constraints(), child->margin(), background);
    child_layout->initialize_with_context(child_context);
}

void WidgetBase::RenderContentResource::initialize()
{
    if (is_valid()) return;

    RenderContext context{ m_widget->render_target(), m_widget->render_bounds_resource()->get() };
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

    if (m_background_widget != nullptr) {
        auto background_context = render_context.create_subcontext(m_background_widget->render_bounds_resource()->get());
        auto background_content = static_pointer_cast<RenderContentResource>(m_background_widget->render_content());
        background_content->initialize_with_context(background_context);
    }

    m_widget->render(render_context);
    auto hr = render_context.render_target()->Flush();
    Logger.at(NAMEOF(WidgetBase::RenderContentResource::initialize_with_context)).at(NAMEOF(ID2D1RenderTarget::Flush)).log_error(hr);

    m_widget->for_each_child([&render_context](WidgetBase* child) {
        auto child_render_context = render_context.create_subcontext(child->render_bounds_resource()->get());
        auto child_render_content = static_pointer_cast<RenderContentResource>(child->render_content());
        child_render_content->initialize_with_context(child_render_context);
        });

    m_background_widget = m_widget->m_layout->get().background_widget();
    mark_valid();
}

void WidgetBase::RenderContentResource::discard() { 
    if (m_background_widget != nullptr) {
        m_background_widget->render_content()->discard();
    }
    mark_invalid();
}
