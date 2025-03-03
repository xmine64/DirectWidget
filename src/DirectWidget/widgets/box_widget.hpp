// box_widget.hpp: Box widget definition

#pragma once

#include <d2d1.h>
#include <d2d1helper.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/resource.hpp"
#include "../core/interop.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class BoxWidget : public WidgetBase {
        public:

            // properties

            static property_ptr<D2D1_COLOR_F> BackgroundColorProperty;
            static property_ptr<D2D1_COLOR_F> StrokeColorProperty;
            static property_ptr<float> StrokeWidthProperty;

            D2D1_COLOR_F background_color() const { return get_property(BackgroundColorProperty); }
            void set_background_color(const D2D1_COLOR_F& color) { set_property(BackgroundColorProperty, color); }

            D2D1_COLOR_F stroke_color() const { return get_property(StrokeColorProperty); }
            void set_stroke_color(const D2D1_COLOR_F& color) { set_property(StrokeColorProperty, color); }

            float stroke_width() const { return get_property(StrokeWidthProperty); }
            void set_stroke_width(float size) { set_property(StrokeWidthProperty, size); }

            // resources

            BoxWidget();

        protected:
            void render(const RenderContext& context) const override;

        private:
            static const LogContext m_log;

            static Interop::com_resource_ptr<ID2D1SolidColorBrush> FillBrushResource;
            static Interop::com_resource_ptr<ID2D1SolidColorBrush> StrokeBrushResource;
        };

    }
}