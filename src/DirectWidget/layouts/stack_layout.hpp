// stack_layout.hpp: StackLayout definition
// StackLayout is a layout widget that arrenges its children in a horizontal or vertical stack.

#pragma once

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "layout_widget.hpp"

namespace DirectWidget {
    namespace Layouts {

        enum STACK_LAYOUT_ORIENTATION {
            STACK_LAYOUT_HORIZONTAL,
            STACK_LAYOUT_VERTICAL
        };

        class StackLayout : public LayoutWidgetBase
        {
        public:

            // properties
            static property_ptr<STACK_LAYOUT_ORIENTATION> OrientationProperty;

            STACK_LAYOUT_ORIENTATION get_orientation() const { return get_property(OrientationProperty); }
            void set_orientation(STACK_LAYOUT_ORIENTATION value) { set_property(OrientationProperty, value); }
            
            StackLayout() {
                register_property(OrientationProperty, m_orientation);
            }

            // layout

            void layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) override;

            SIZE_F measure(const SIZE_F& available_size) const override;

        private:

            STACK_LAYOUT_ORIENTATION m_orientation;
        };

    }
}