// box_widget.hpp: Box widget definition

#pragma once

#include <d2d1.h>
#include <d2d1helper.h>

#include "../core/foundation.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class BoxWidget : public WidgetBase {
        public:

            // properties

            D2D1::ColorF background_color() const { return m_background_color; }
            void set_background_color(const D2D1::ColorF& color) { m_background_color = color; }

            D2D1::ColorF stroke_color() const { return m_stroke_color; }
            void set_stroke_color(const D2D1::ColorF& color) { m_stroke_color = color; }

            // render

            void create_resources() override;

        protected:

            void on_render() const override;

        private:

            D2D1::ColorF m_background_color{D2D1::ColorF::White};
            D2D1::ColorF m_stroke_color{D2D1::ColorF::Black};

            com_ptr<ID2D1Brush> m_background_brush{};
            com_ptr<ID2D1Brush> m_stroke_brush{};
        };

    }
}