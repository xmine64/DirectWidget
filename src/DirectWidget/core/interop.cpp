#include "interop.hpp"

#include <comdef.h>
#include <d2d1.h>

#include "element_base.hpp"
#include "widget.hpp"

using namespace DirectWidget;
using namespace Interop;

HRESULT SolidColorBrushResource::initialize(const ElementBase* owner, com_ptr<ID2D1SolidColorBrush>& resource) {
    auto& render_target = static_cast<const WidgetBase*>(owner)->render_target();
    return render_target->CreateSolidColorBrush(m_color_property->get_value(owner), &resource);
}
