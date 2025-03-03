// composite_widget.hpp: Composite widget definition

#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "../core/foundation.hpp"
#include "../core/property.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class CompositeWidget : public WidgetBase {

        public:

            // properties
            
            static collection_property_ptr<std::shared_ptr<WidgetBase>> ChildrenProperty;

            void add_child(std::shared_ptr<WidgetBase> widget);
            void remove_child(std::shared_ptr<WidgetBase> widget);

            CompositeWidget() {
                register_dependency(ChildrenProperty);
            }

            // layout
            
            void layout(LayoutContext& context) const override;

            SIZE_F measure(const SIZE_F& available_size) const;

            // render

            // interaction

            bool handle_pointer_hover(D2D1_POINT_2F point) override;
            bool handle_pointer_press(D2D1_POINT_2F point) override;
            bool handle_pointer_release(D2D1_POINT_2F point) override;

        protected:
            void for_each_child(std::function<void(WidgetBase*)> callback) const override {
                auto& children = ChildrenProperty->get_values(this);
                for (auto& child : children) {
                    callback(child.get());
                }
            }
        };

    }
}
