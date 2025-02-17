// text_widget.hpp: Text widget definition

#pragma once

#include <Windows.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "../core/foundation.hpp"
#include "../core/app.hpp"
#include "../core/property.hpp"
#include "../core/resource.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class TextWidget : public WidgetBase
        {
        public:

            // properties

            static property_ptr<PCWSTR> TextProperty;
            static property_ptr<PCWSTR> FontFamilyProperty;
            static property_ptr<float> FontSizeProperty;
            static property_ptr<D2D1::ColorF> ColorProperty;
            static property_ptr<DWRITE_FONT_WEIGHT> FontWeightProperty;
            static property_ptr<DWRITE_TEXT_ALIGNMENT> TextAlignmentProperty;
            static property_ptr<DWRITE_PARAGRAPH_ALIGNMENT> ParagraphAlignmentProperty;

            const PCWSTR& text() const { return get_property(TextProperty); }
            void set_text(PCWSTR text) { set_property(TextProperty, text); }
            
            const PCWSTR& font_family() const { return get_property(FontFamilyProperty); }
            void set_font_family(PCWSTR font_family) { set_property(FontFamilyProperty, font_family); }
            
            float font_size() const { return get_property(FontSizeProperty); }
            void set_font_size(float size) { set_property(FontSizeProperty, size); }
            
            D2D1::ColorF color() const { return get_property(ColorProperty); }
            void set_color(D2D1::ColorF color) { set_property(ColorProperty, color); }
            
            DWRITE_FONT_WEIGHT font_weight() const { return get_property(FontWeightProperty); }
            void set_font_weight(DWRITE_FONT_WEIGHT weight) { set_property(FontWeightProperty, weight); }
            
            DWRITE_TEXT_ALIGNMENT text_alignment() const { return get_property(TextAlignmentProperty); }
            void set_text_alignment(DWRITE_TEXT_ALIGNMENT alignment) { set_property(TextAlignmentProperty, alignment); }
            
            DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment() const { return get_property(ParagraphAlignmentProperty); }
            void set_paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT alignment) { set_property(ParagraphAlignmentProperty, alignment); }

            TextWidget();

            // layout

            SIZE_F measure(const SIZE_F& available_size) const override;

        protected:

            void render(const RenderContext& context) const override;

        private:

            static const LogContext Logger;

            PCWSTR m_text{};
            PCWSTR m_font_family = L"Segoe UI";
            float m_size = 12.0;
            D2D1::ColorF m_color = D2D1::ColorF(D2D1::ColorF::Black);
            DWRITE_FONT_WEIGHT m_weight = DWRITE_FONT_WEIGHT_NORMAL;
            DWRITE_TEXT_ALIGNMENT m_alignment = DWRITE_TEXT_ALIGNMENT_LEADING;
            DWRITE_PARAGRAPH_ALIGNMENT m_vertical_alignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;

            com_resource_ptr<ID2D1Brush> m_text_fill;
            com_resource_ptr<IDWriteTextFormat> m_text_format;
            com_resource_ptr<IDWriteTextLayout> m_text_layout;

            class TextLayoutUpdater : public PropertyOwnerChangeListenerBase {

            public:

                void on_property_changed(sender_ptr sender, property_base_ptr property) override;

            };
        };

    }
}