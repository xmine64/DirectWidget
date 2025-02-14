// box_widget.hpp: Box widget definition

#pragma once

#include <d2d1.h>
#include <d2d1helper.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class BoxWidget : public WidgetBase {
        public:

            // properties

            static property_ptr<D2D1::ColorF> BackgroundColorProperty;
            static property_ptr<D2D1::ColorF> StrokeColorProperty;

            D2D1::ColorF background_color() const { return get_property(BackgroundColorProperty); }
            void set_background_color(const D2D1::ColorF& color) { set_property(BackgroundColorProperty, color); }

            D2D1::ColorF stroke_color() const { return get_property(StrokeColorProperty); }
            void set_stroke_color(const D2D1::ColorF& color) { set_property(StrokeColorProperty, color); }

            BoxWidget() {
                register_property(BackgroundColorProperty, m_background_color);
                register_property(StrokeColorProperty, m_stroke_color);
            }

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