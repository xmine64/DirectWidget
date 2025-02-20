#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "foundation.hpp"
#include "property.hpp"

namespace DirectWidget {
    class ElementBase {
    public:
        virtual ~ElementBase() = default;

        void add_listener(const owner_listener_ptr& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const owner_listener_ptr& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

    protected:
        template<typename P>
        void register_property(property_ptr<P> property, P& field) {
            m_properties[property] = &field;
            field = property->get_default_value();
        }

        template<typename C>
        void register_collection(collection_property_ptr<C> property, std::vector<C>& field) {
            m_properties[property] = &field;
        }

        void notify_change(property_base_ptr property) {
            for (const auto& listener : m_listeners) {
                listener->on_property_changed(this, property);
            }

            property->notify_change(this);
        }

        template<typename P>
        void set_property(const property_ptr<P>& property, const P& value) {
            for (auto& [prop, ptr] : m_properties) {
                if (prop.get() == property.get()) {
                    auto old_value = *static_cast<P*>(ptr);
                    *static_cast<P*>(ptr) = value;

                    property->notify_change(this, old_value, value);
                    notify_change(property);
                    return;
                }
            }
        }

        template<typename P>
        const P& get_property(const property_ptr<P>& property) const {
            for (auto& [prop, ptr] : m_properties) {
                if (prop == property) {
                    return *static_cast<P*>(ptr);
                }
            }
            return property->get_default_value();
        }

        template<typename C>
        void add_to_collection(collection_property_ptr<C> property, const C& value) {
            for (auto& [prop, ptr] : m_properties) {
                if (prop == property) {
                    auto& collection = *static_cast<std::vector<C>*>(ptr);
                    collection.push_back(value);

                    property->notify_change(this, value, true);
                    notify_change(property);
                    return;
                }
            }
        }

        template<typename C>
        void remove_from_collection(collection_property_ptr<C> property, const C& value) {
            for (auto& [prop, ptr] : m_properties) {
                if (prop == property) {
                    auto& collection = *static_cast<std::vector<C>*>(ptr);
                    collection.erase(std::remove(collection.begin(), collection.end(), value), collection.end());

                    property->notify_change(this, value, false);
                    notify_change(property);
                    return;
                }
            }
        }

        template<typename C>
        const std::vector<C>& get_collection_values(collection_property_ptr<C> property) const {
            for (auto& [prop, ptr] : m_properties) {
                if (prop == property) {
                    return *static_cast<std::vector<C>*>(ptr);
                }
            }
            return std::vector<C>();
        }

    private:
        std::unordered_map<std::shared_ptr<PropertyBase>, void*> m_properties;
        std::vector<owner_listener_ptr> m_listeners;
    };
}