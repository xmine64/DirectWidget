// composite_widget.hpp: Composite widget definition

#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "../core/foundation.hpp"
#include "../core/widget.hpp"

namespace DirectWidget {
    namespace Widgets {

        class CompositeWidget : public WidgetBase {

        public:

            // properties
            
            void add_widget(std::shared_ptr<WidgetBase> widget) { m_children.push_back(widget); }

            // layout
            
            void layout(const BOUNDS_F& constraints, BOUNDS_F& layout_bounds, BOUNDS_F& render_bounds) const override;

            SIZE_F measure(const SIZE_F& available_size) const;

            // render

        protected:

            void for_each_child(std::function<void(WidgetBase*)> callback) const override {
                for (auto& child : m_children) {
                    callback(child.get());
                }
            }

        private:
            std::vector<std::shared_ptr<WidgetBase>> m_children{};

        };

    }
}
