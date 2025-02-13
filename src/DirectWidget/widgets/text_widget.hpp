// text_widget.hpp: Text widget definition

#pragma once

#include <Windows.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class TextWidget : public WidgetBase
        {
        public:

            // properties

            const PCWSTR& text() const { return m_text; }
            void set_text(PCWSTR text) { m_text = text; }
            
            const PCWSTR& font_family() const { return m_font_family; }
            void set_font_family(PCWSTR font_family) { m_font_family = font_family; }
            
            float font_size() const { return m_size; }
            void set_font_size(float size) { m_size = size; }
            
            D2D1::ColorF color() const { return m_color; }
            void set_color(D2D1::ColorF color) { m_color = color; }
            
            DWRITE_FONT_WEIGHT font_weight() const { return m_weight; }
            void set_font_weight(DWRITE_FONT_WEIGHT weight) { m_weight = weight; }
            
            DWRITE_TEXT_ALIGNMENT text_alignment() const { return m_alignment; }
            void set_text_alignment(DWRITE_TEXT_ALIGNMENT alignment) { m_alignment = alignment; }
            
            DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment() const { return m_vertical_alignment; }
            void set_paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT alignment) { m_vertical_alignment = alignment; }

            // layout

            SIZE_F measure(const SIZE_F& available_size) const override;

            void on_layout_finalized(const BOUNDS_F& render_bounds) override;

            // render

            void create_resources() override;
            void discard_resources() override;

        protected:

            void on_render() const override;

        private:
            PCWSTR m_text{};
            PCWSTR m_font_family = L"Segoe UI";
            float m_size = 12.0;
            D2D1::ColorF m_color = D2D1::ColorF(D2D1::ColorF::Black);
            DWRITE_FONT_WEIGHT m_weight = DWRITE_FONT_WEIGHT_NORMAL;
            DWRITE_TEXT_ALIGNMENT m_alignment = DWRITE_TEXT_ALIGNMENT_LEADING;
            DWRITE_PARAGRAPH_ALIGNMENT m_vertical_alignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;

            com_ptr<IDWriteTextFormat> m_text_format = NULL;
            com_ptr<IDWriteTextLayout> m_text_layout = NULL;

            com_ptr<ID2D1SolidColorBrush> m_brush = NULL;
        };

    }
}