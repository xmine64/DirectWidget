// layout_widget.hpp: LayoutWidgetBase definition
// LayoutWidgetBase provides abstraction for layout widgets

#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Layouts {

        typedef struct {

            std::shared_ptr<WidgetBase> widget;

            SIZE_F measure;

            SIZE_F render_size;
            SIZE_F layout_size;

            BOUNDS_F render_bounds;
            BOUNDS_F layout_bounds;

        } LAYOUT_NODE;

        class LayoutWidgetBase : public WidgetBase {
        public:

            // properties

            static collection_property_ptr<std::shared_ptr<WidgetBase>> ChildrenProperty;

            void add_child(std::shared_ptr<WidgetBase> widget) { add_to_collection(ChildrenProperty, widget); }
            void remove_child(std::shared_ptr<WidgetBase> widget) { remove_from_collection(ChildrenProperty, widget); }

            // layout

            // render

            void create_resources() override;
            void discard_resources() override;

            // interaction

            bool handle_pointer_hover(D2D1_POINT_2F point) override;
            bool handle_pointer_press(D2D1_POINT_2F point) override;
            bool handle_pointer_release(D2D1_POINT_2F point) override;

        protected:

            LayoutWidgetBase() {
                LayoutWidgetChildrenListener::register_listener();

                register_collection(ChildrenProperty, m_children);
            }

            void for_each_child(std::function<void(WidgetBase*)> callback) const override { for (auto& node : m_nodes) { callback(node->widget.get()); } }

            std::vector<std::unique_ptr<LAYOUT_NODE>> m_nodes;

        private:

            std::vector<std::shared_ptr<WidgetBase>> m_children;

            class LayoutWidgetChildrenListener : public CollectionListener<std::shared_ptr<WidgetBase>> {

            public:

                static void register_listener();

                void on_collection_changed(sender_ptr sender, property_token property, const std::shared_ptr<WidgetBase>& widget, bool add_or_remove);

            };
        };

    }
}