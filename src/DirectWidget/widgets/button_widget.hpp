// button_widget.hpp: Button widget class definition

#pragma once

#include <functional>
#include <memory>

#include <Windows.h>
#include <d2d1.h>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "composite_widget.hpp"
#include "text_widget.hpp"
#include "box_widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class ButtonWidget : public CompositeWidget
        {
        public:

            ButtonWidget();

            // properties

            static property_ptr<PCWSTR> TextProperty;
            static property_ptr<BOUNDS_F> PaddingProperty;
            static property_ptr<D2D1_COLOR_F> ForegroundColorProperty;
            static property_ptr<D2D1_COLOR_F> StrokeColorProperty;
            static property_ptr<D2D1_COLOR_F> BackgroundColorProperty;
            static property_ptr<D2D1_COLOR_F> HoverColorProperty;
            static property_ptr<D2D1_COLOR_F> PressedColorProperty;

            const PCWSTR& text() const { return get_property(TextProperty); }
            void set_text(const PCWSTR& text) { set_property(TextProperty, text); }

            BOUNDS_F padding() { return get_property(PaddingProperty); }
            void set_padding(const BOUNDS_F& padding) { set_property(PaddingProperty, padding); }

            D2D1_COLOR_F foreground_color() const { return get_property(ForegroundColorProperty); }
            void set_foreground_color(D2D1_COLOR_F color) { set_property(ForegroundColorProperty, color); }

            D2D1_COLOR_F stroke_color() const { return get_property(StrokeColorProperty); }
            void set_stroke_color(D2D1_COLOR_F color) { set_property(StrokeColorProperty, color); }

            D2D1_COLOR_F background_color() const { return get_property(BackgroundColorProperty); }
            void set_background_color(D2D1_COLOR_F color) { set_property(BackgroundColorProperty, color); }

            D2D1_COLOR_F hover_color() const { return get_property(HoverColorProperty); }
            void set_hover_color(D2D1_COLOR_F color) { set_property(HoverColorProperty, color); }

            D2D1_COLOR_F pressed_color() const { return get_property(PressedColorProperty); }
            void set_pressed_color(D2D1_COLOR_F color) { set_property(PressedColorProperty, color); }

            void set_click_handler(std::function<void()> handler) { m_click_handler = handler; }

            // layout

            //SIZE_F measure(const SIZE_F& available_size) const override;

            // render

            void create_resources() override;

            // interaction

            bool handle_pointer_hover(D2D1_POINT_2F point) override;
            bool handle_pointer_press(D2D1_POINT_2F point) override;
            bool handle_pointer_release(D2D1_POINT_2F point) override;

        private:
            std::shared_ptr<TextWidget> m_text_widget;
            std::shared_ptr<BoxWidget> m_box_widget;

            std::function<void()> m_click_handler;
        };

    }
}