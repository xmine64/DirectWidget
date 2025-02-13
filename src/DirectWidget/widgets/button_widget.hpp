// button_widget.hpp: Button widget class definition

#pragma once

#include <functional>
#include <memory>

#include <Windows.h>
#include <d2d1helper.h>

#include "../core/foundation.hpp"
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

            const PCWSTR& text() const { return m_text; }
            void set_text(const PCWSTR& text) { m_text = text; }

            BOUNDS_F padding() { return m_padding; }
            void set_padding(const BOUNDS_F& padding) { m_padding = padding; }

            D2D1::ColorF foreground_color() const { return m_foreground_color; }
            void set_foreground_color(D2D1::ColorF color) { m_foreground_color = color; }

            D2D1::ColorF stroke_color() const { return m_stroke_color; }
            void set_stroke_color(D2D1::ColorF color) { m_stroke_color = color; }

            D2D1::ColorF background_color() const { return m_background_color; }
            void set_background_color(D2D1::ColorF color) { m_background_color = color; }

            D2D1::ColorF hover_color() const { return m_hover_color; }
            void set_hover_color(D2D1::ColorF color) { m_hover_color = color; }

            D2D1::ColorF pressed_color() const { return m_pressed_color; }
            void set_pressed_color(D2D1::ColorF color) { m_pressed_color = color; }

            void set_click_handler(std::function<void()> handler) { m_click_handler = handler; }

            // layout

            SIZE_F measure(const SIZE_F& available_size) const override;

            // render

            void create_resources() override;

            // interaction

            bool handle_pointer_hover(D2D1_POINT_2F point) override;
            bool handle_pointer_press(D2D1_POINT_2F point) override;
            bool handle_pointer_release(D2D1_POINT_2F point) override;

        private:

            std::shared_ptr<TextWidget> m_text_widget;
            std::shared_ptr<BoxWidget> m_box_widget;

            PCWSTR m_text = L"Button";

            BOUNDS_F m_padding{ 4.0f, 4.0f, 4.0f, 4.0f };

            D2D1::ColorF m_foreground_color = D2D1::ColorF(D2D1::ColorF::Black);
            D2D1::ColorF m_stroke_color = D2D1::ColorF(D2D1::ColorF::LightSlateGray);
            D2D1::ColorF m_background_color = D2D1::ColorF(D2D1::ColorF::LightGray);
            D2D1::ColorF m_hover_color = D2D1::ColorF(D2D1::ColorF::DimGray);
            D2D1::ColorF m_pressed_color = D2D1::ColorF(D2D1::ColorF::Gray);

            std::function<void()> m_click_handler;
        };

    }
}