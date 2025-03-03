// base_widget.cpp: BaseWidget implementation

#include <memory>
#include <unordered_map>

#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <comdef.h>

#include "foundation.hpp"
#include "app.hpp"
#include "element_base.hpp"
#include "window.hpp"
#include "widget.hpp"
#include "dependency.hpp"
#include "property.hpp"
#include "resource.hpp"
#include "interop.hpp"

using namespace DirectWidget;
using namespace DirectWidget::Interop;

const LogContext WidgetBase::Logger{ NAMEOF(WidgetBase) };
const LogContext RenderContext::Logger{ NAMEOF(RenderContext) };

class WidgetBase::WidgetMeasureResource : public BasicTypeResource<SIZE_F> {
protected:
    bool initialize(const ElementBase* owner, SIZE_F& resource) override {
        SIZE_F available_size{ WidgetBase::MaxSizeProperty->get_value(owner) };

        auto& size = WidgetBase::SizeProperty->get_value(owner);
        if (size.width > 0) {
            available_size.width = min(available_size.width, size.width);
        }

        if (size.height > 0) {
            available_size.height = min(available_size.height, size.height);
        }

        auto& margin = WidgetBase::MarginProperty->get_value(owner);
        auto margin_width = margin.left + margin.right;
        auto margin_height = margin.top + margin.bottom;

        available_size.width = max(available_size.width - margin_width, 0);
        available_size.height =max(available_size.height - margin_height, 0);

        auto content_size = static_cast<const WidgetBase*>(owner)->measure(available_size);

        resource.width = content_size.width + margin_width;
        resource.height = content_size.height + margin_height;
        return true;
    }
};

class WidgetBase::WidgetRenderBoundsResource : public BasicTypeResource<BOUNDS_F> {
protected:
    bool initialize(const ElementBase* owner, BOUNDS_F& resource) override {
        auto& layout = LayoutResource->get_or_initialize_resource(owner);
        resource = layout.render_bounds();
        return true;
    }
};

class WidgetBase::WidgetRenderGeometryResource : public Interop::ComResource<ID2D1Geometry> {
protected:
    HRESULT initialize(const ElementBase* owner, com_ptr<ID2D1Geometry>& resource) {
        auto& render_bounds = WidgetBase::RenderBoundsResource->get_or_initialize_resource(owner);
        auto rect = D2D1::RectF(render_bounds.left, render_bounds.top, render_bounds.right, render_bounds.bottom);

        auto& d2d = DirectWidget::Application::instance()->d2d();
        auto hr = d2d->CreateRectangleGeometry(rect, &reinterpret_cast<ID2D1RectangleGeometry*&>(resource));
        Logger.at(NAMEOF(m_render_geometry)).at(NAMEOF(ID2D1Factory::CreateRectangleGeometry)).fatal_exit(hr);
        return hr;
    }
};

class WidgetBase::WidgetLayoutResource : public Resource<LayoutContext> {
public:
    void register_owner(const ElementBase* owner) override {
        ResourceBase::register_owner(owner);
        m_resources[owner] = LayoutContext({ 0,0 }, { 0,0,0,0 }, { 0,0,0,0 });
    }

    void remove_owner(const ElementBase* owner) override {
        ResourceBase::remove_owner(owner);
        m_resources.erase(owner);
    }

    const LayoutContext& get_resource(const ElementBase* owner) const {
        auto it = m_resources.find(owner);
        if (it != m_resources.end()) {
            return it->second;
        }
        return LayoutContext({ 0,0 }, { 0,0,0,0 }, { 0,0,0,0 });
    }

    bool initialize(const ElementBase* owner) override {
        if (is_valid(owner) == true) return true;

        auto& resource = m_resources[owner];
        resource = LayoutContext(
            WidgetBase::MeasureResource->get_or_initialize_resource(owner),
            WidgetBase::ConstraintsProperty->get_value(owner),
            WidgetBase::MarginProperty->get_value(owner),
            resource.background_widget());
        static_cast<const WidgetBase*>(owner)->layout(resource);
        return true;
    }

    void initialize_with_context(const ElementBase* owner, const LayoutContext& context)
    {
        if (is_valid(owner) == true) return;

        auto& resource = m_resources[owner];
        resource = context;
        static_cast<const WidgetBase*>(owner)->layout(resource);
    }

    void discard(const ElementBase* owner) override {}

private:
    std::unordered_map<const ElementBase*, LayoutContext> m_resources;
};

class WidgetBase::WidgetRenderContentResource : public ResourceBase {
public:
    bool initialize(const ElementBase* owner) override {
        RenderContext context{
            static_cast<const WidgetBase*>(owner)->render_target(),
            WidgetBase::RenderBoundsResource->get_or_initialize_resource(owner)
        };
        initialize_with_context(owner, context);

        // TODO:
        // if (hr == D2DERR_RECREATE_TARGET)

        return true;
    }

    void initialize_with_context(const ElementBase* owner, const RenderContext& render_context) {
        auto& render_bounds = render_context.render_bounds();
        if (render_bounds.right - render_bounds.left <= 0 ||
            render_bounds.bottom - render_bounds.top <= 0)
            return;

        auto widget = static_cast<const WidgetBase*>(owner);

        if (is_valid(owner) == false) {
            auto& background_widget = m_background_widgets[owner];
            if (background_widget != nullptr) {
                auto background_context = render_context.create_subcontext(WidgetBase::RenderBoundsResource->get_or_initialize_resource(background_widget.get()));
                initialize_with_context(background_widget.get(), background_context);
            }

            widget->render(render_context);
            auto hr = render_context.render_target()->Flush();
            Logger.at(NAMEOF(WidgetBase::RenderContentResource::initialize_with_context)).at(NAMEOF(ID2D1RenderTarget::Flush)).log_error(hr);

            m_background_widgets[owner] = WidgetBase::LayoutResource->get_resource(owner).background_widget();

            if (Application::instance()->is_debug()) {
                static_cast<const WidgetBase*>(owner)->render_debug_layout(render_context.render_target());
            }
        }

        widget->for_each_child([this, &render_context](WidgetBase* child) {
            auto child_render_context = render_context.create_subcontext(WidgetBase::RenderBoundsResource->get_or_initialize_resource(child));
            initialize_with_context(child, child_render_context);
            });
    }

    void discard(const ElementBase* owner) override {
        auto& background_widget = m_background_widgets[owner];
        if (background_widget != nullptr) {
            background_widget->discard_frame();
        }
    }

private:
    std::unordered_map<const ElementBase*, widget_ptr> m_background_widgets;
};

class WidgetBase::WidgetRenderTargetProperty : public PropertyBase {
public:
    void notify_change(ElementBase* owner) {
        DependencyBase::notify_updated(owner, NotificationArgument(NotificationType::Updated, this));
    }
};

//
// Properties
//

property_ptr<SIZE_F> WidgetBase::SizeProperty = make_property(SIZE_F{ 0, 0 });
property_ptr<BOUNDS_F> WidgetBase::MarginProperty = make_property(BOUNDS_F{ 0, 0, 0, 0 });
property_ptr<WidgetAlignment> WidgetBase::VerticalAlignmentProperty = make_property(WidgetAlignment::Center);
property_ptr<WidgetAlignment> WidgetBase::HorizontalAlignmentProperty = make_property(WidgetAlignment::Center);

property_ptr<SIZE_F> WidgetBase::MaxSizeProperty = make_property<SIZE_F>({ 0,0 });
property_ptr<BOUNDS_F> WidgetBase::ConstraintsProperty = make_property<BOUNDS_F>({ 0,0,0,0 });

property_base_ptr WidgetBase::RenderTargetProperty = std::make_shared<PropertyBase>();

//
// Resources
//

resource_ptr<SIZE_F> WidgetBase::MeasureResource = std::make_shared<WidgetMeasureResource>();
resource_ptr<LayoutContext> WidgetBase::LayoutResource = std::make_shared<WidgetLayoutResource>();
resource_ptr<BOUNDS_F> WidgetBase::RenderBoundsResource = std::make_shared<WidgetRenderBoundsResource>();
Interop::com_resource_ptr<ID2D1Geometry> WidgetBase::RenderGeometryResource = std::make_shared<WidgetRenderGeometryResource>();

resource_base_ptr WidgetBase::RenderContentResource = std::make_shared<WidgetRenderContentResource>();

resource_ptr<float> WidgetBase::ScaleResource = std::make_shared<InheritedResource<float>>(Window::ScaleResource);

WidgetBase::WidgetBase() {
    register_dependency(SizeProperty);
    register_dependency(MarginProperty);
    register_dependency(VerticalAlignmentProperty);
    register_dependency(HorizontalAlignmentProperty);

    register_dependency(MaxSizeProperty);
    register_dependency(ConstraintsProperty);

    register_dependency(RenderTargetProperty);

    register_dependency(MeasureResource);
    register_dependency(LayoutResource);
    register_dependency(RenderBoundsResource);
    register_dependency(RenderGeometryResource);
    register_dependency(RenderContentResource);
    register_dependency(ScaleResource);
}

void WidgetBase::layout(LayoutContext& context) const
{
    // Align widget in the given bounds

    auto& size = context.measure();
    auto& constraints = context.constraints();
    auto& layout_bounds = context.layout_bounds();

    switch (horizontal_alignment()) {
    case WidgetAlignment::Start:
        layout_bounds.left = constraints.left;
        layout_bounds.right = layout_bounds.left + size.width;
        break;

    case WidgetAlignment::Center:
    {
        auto free_width = (constraints.right - constraints.left) - size.width;
        layout_bounds.left = constraints.left + max(0.0f, free_width / 2);
        layout_bounds.right = layout_bounds.left + size.width;
    }
    break;

    case WidgetAlignment::Stretch:
        layout_bounds.left = constraints.left;
        layout_bounds.right = constraints.right;
        break;

    case WidgetAlignment::End:
        layout_bounds.right = constraints.right;
        layout_bounds.left = layout_bounds.right - size.width;
        break;
    }

    switch (vertical_alignment()) {
    case WidgetAlignment::Start:
        layout_bounds.top = constraints.top;
        layout_bounds.bottom = layout_bounds.top + size.height;
        break;

    case WidgetAlignment::Center:
    {
        auto free_height = (constraints.bottom - constraints.top) - size.height;
        layout_bounds.top = constraints.top + max(0.0f, free_height / 2);
        layout_bounds.bottom = layout_bounds.top + size.height;
    }
    break;

    case WidgetAlignment::Stretch:
        layout_bounds.top = constraints.top;
        layout_bounds.bottom = constraints.bottom;
        break;

    case WidgetAlignment::End:
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

    auto& layout_bounds = LayoutResource->get_resource(this).layout_bounds();
    auto& render_bounds = RenderBoundsResource->get_resource(this);

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

    static_pointer_cast<WidgetRenderTargetProperty>(RenderTargetProperty)->notify_change(this);
}

void WidgetBase::detach_render_target()
{
    discard_resources();
    m_render_target = nullptr;
    for_each_child([](WidgetBase* widget) {
        widget->detach_render_target();
        });

    static_pointer_cast<WidgetRenderTargetProperty>(RenderTargetProperty)->notify_change(this);
}

bool WidgetBase::hit_test(D2D1_POINT_2F point) {
    BOOL result = false;
    auto hr = WidgetBase::RenderGeometryResource->get_or_initialize_resource(this)->FillContainsPoint(point, D2D1::Matrix3x2F::Identity(), &result);
    Logger.at(NAMEOF(hit_test)).at(NAMEOF(ID2D1Geometry::FillContainsPoint)).log_error(hr);
    return result;
}

void LayoutContext::layout_child(const widget_ptr& child, const BOUNDS_F& constraints, const widget_ptr& background) const {
    child->set_constraints(constraints);
    auto child_context = create_subcontext(WidgetBase::MeasureResource->get_or_initialize_resource(child.get()), child->constraints(), child->margin(), background);
    static_pointer_cast<WidgetBase::WidgetLayoutResource>(WidgetBase::LayoutResource)->initialize_with_context(child.get(), child_context);
}
