// layout_widget.hpp: LayoutWidgetBase definition
// LayoutWidgetBase provides abstraction for layout widgets

#pragma once

#include <memory>
#include <vector>

#include "../core/foundation.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Layouts {

        typedef struct {

            std::unique_ptr<WidgetBase> widget;

            SIZE_F measure;

            SIZE_F render_size;
            SIZE_F layout_size;

            BOUNDS_F render_bounds;
            BOUNDS_F layout_bounds;

        } LAYOUT_NODE;

        class LayoutWidgetBase : public WidgetBase {
        public:

            // properties

            void add_widget(std::unique_ptr<WidgetBase> widget) { auto node = std::make_unique<LAYOUT_NODE>(); node->widget = std::move(widget); m_nodes.push_back(std::move(node)); }

            // layout

            // render

            void create_resources() override;
            void discard_resources() override;

            // interaction

            bool handle_pointer_hover(D2D1_POINT_2F point) override;
            bool handle_pointer_press(D2D1_POINT_2F point) override;
            bool handle_pointer_release(D2D1_POINT_2F point) override;

        protected:

            void for_each_child(std::function<void(WidgetBase*)> callback) const override { for (auto& node : m_nodes) { callback(node->widget.get()); } }

            std::vector<std::unique_ptr<LAYOUT_NODE>> m_nodes;
        };

    }
}