// stack_layout.hpp: StackLayout definition
// StackLayout is a layout widget that arrenges its children in a horizontal or vertical stack.

#pragma once

#include "../core/foundation.hpp"
#include "layout_widget.hpp"

namespace DirectWidget {
    namespace Layouts {

        class StackLayout : public LayoutWidgetBase
        {
        public:

            // properties

            bool is_horizontal() const { return m_orientation; }
            void set_horizontal() { m_orientation = true; }

            bool is_vertical() const { return !m_orientation; }
            void set_vertical() { m_orientation = false; }

            // layout

            void layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) const override;

            SIZE_F measure(const SIZE_F& available_size) const override;

        private:
            bool m_orientation = false;
        };

    }
}