// property.hpp: Mutable property definition
// Mutable property can be used to define a property on a PropertyOwner with change notification

#pragma once

#include <memory>
#include <vector>

#include "foundation.hpp"

namespace DirectWidget {

    // Listeners

    class PropertyListenerBase {
    public:
        virtual ~PropertyListenerBase() = default;
        virtual void on_property_changed(ElementBase* sender, property_token property) = 0;
    };

    template <typename T>
    class PropertyValueListener : public PropertyListenerBase {
    public:
        void on_property_changed(ElementBase* sender, property_token property) override {}
        virtual void on_property_changed(ElementBase* sender, property_token property, const T& old_value, const T& new_value) = 0;
    };

    template <typename T>
    class CollectionListener : public PropertyListenerBase {
    public:
        void on_property_changed(ElementBase* sender, property_token property) override {}
        virtual void on_collection_changed(ElementBase* sender, property_token property, const T& value, bool add_or_remove) = 0;
    };

    class PropertyOwnerChangeListenerBase {
    public:
        virtual ~PropertyOwnerChangeListenerBase() = default;
        virtual void on_property_changed(ElementBase* sender, property_base_ptr property) = 0;
    };

    // Properties

    class PropertyBase {
    public:
        virtual ~PropertyBase() = default;

        void add_listener(const property_listener_ptr& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const property_listener_ptr& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

    protected:
        void notify_change(ElementBase* sender) {
            for (auto& listener : m_listeners) {
                listener->on_property_changed(sender, this);
            }
        }

    private:
        std::vector<property_listener_ptr> m_listeners;

        friend ElementBase;
    };

    template <typename T>
    class Property : public PropertyBase {
    public:
        Property(const T& default_value) : m_default_value(default_value) {}

        const T& get_default_value() const {
            return m_default_value;
        }

        void add_listener(const property_value_listener_ptr<T>& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const property_value_listener_ptr<T>& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

    protected:
        void notify_change(ElementBase* sender, const T& old_value, const T& new_value) {
            for (auto& listener : m_listeners) {
                listener->on_property_changed(sender, this, old_value, new_value);
                listener->on_property_changed(sender, this);
            }

            PropertyBase::notify_change(sender);
        }

    private:
        const T m_default_value;
        std::vector<property_value_listener_ptr<T>> m_listeners;

        friend ElementBase;
    };

    template <typename T>
    class ObservableCollectionProperty : public PropertyBase {
    public:

        void add_listener(const collection_listener_ptr<T>& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const collection_listener_ptr<T>& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

    protected:
        void notify_change(ElementBase* sender, const T& value, bool add_or_remove) {
            for (auto& listener : m_listeners) {
                listener->on_collection_changed(sender, this, value, add_or_remove);
            }

            PropertyBase::notify_change(sender);
        }

    private:
        std::vector<collection_listener_ptr<T>> m_listeners;

        friend ElementBase;
    };

    // helper functions for making properties

    template <typename P>
    inline property_ptr<P> make_property(const P& default_value) {
        return std::make_shared<Property<P>>(default_value);
    }

    template <typename P>
    inline property_ptr<P> make_property(const P& default_value, property_listener_ptr listener) {
        auto& result = make_property(default_value);
        result->add_listener(listener);
        return result;
    }

    template <typename C>
    collection_property_ptr<C> make_collection() {
        return std::make_shared<ObservableCollectionProperty<C>>();
    }
}