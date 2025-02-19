#pragma once

// property.hpp: Mutable property definition
// Mutable property can be used to define a property on a PropertyOwner with change notification

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace DirectWidget {

    // Type forwards

    class PropertyOwnerBase;
    using property_owner_ptr = std::shared_ptr<PropertyOwnerBase>;
    using sender_ptr = PropertyOwnerBase*;

    class PropertyBase;
    using property_base_ptr = std::shared_ptr<PropertyBase>;
    using property_token = PropertyBase*;

    template <typename T>
    class Property;
    template <typename T>
    using property_ptr = std::shared_ptr<Property<T>>;

    template <typename T>
    class ObservableCollectionProperty;
    template <typename T>
    using collection_property_ptr = std::shared_ptr<ObservableCollectionProperty<T>>;

    // Listeners

    class PropertyListenerBase {

    public:
        virtual ~PropertyListenerBase() = default;

        virtual void on_property_changed(sender_ptr sender, property_token property) = 0;

    };

    using property_listener_ptr = std::shared_ptr<PropertyListenerBase>;

    template <typename T>
    class PropertyValueListener : public PropertyListenerBase {

    public:

        void on_property_changed(sender_ptr sender, property_token property) override {}

        virtual void on_property_changed(sender_ptr sender, property_token property, const T& old_value, const T& new_value) = 0;

    };

    template <typename T>
    using property_value_listener_ptr = std::shared_ptr<PropertyValueListener<T>>;

    template <typename T>
    class CollectionListener : public PropertyListenerBase {

    public:

        void on_property_changed(sender_ptr sender, property_token property) override {}

        virtual void on_collection_changed(sender_ptr sender, property_token property, const T& value, bool add_or_remove) = 0;
    };

    template <typename T>
    using collection_listener_ptr = std::shared_ptr<CollectionListener<T>>;


    class PropertyOwnerChangeListenerBase {

    public:
        virtual ~PropertyOwnerChangeListenerBase() = default;

        virtual void on_property_changed(sender_ptr sender, property_base_ptr property) = 0;

    };

    using owner_listener_ptr = std::shared_ptr<PropertyOwnerChangeListenerBase>;

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

        void notify_change(PropertyOwnerBase* sender) {
            for (auto& listener : m_listeners) {
                listener->on_property_changed(sender, this);
            }
        }

    private:

        std::vector<property_listener_ptr> m_listeners;

        friend PropertyOwnerBase;
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

        void notify_change(PropertyOwnerBase* sender, const T& old_value, const T& new_value) {
            for (auto& listener : m_listeners) {
                listener->on_property_changed(sender, this, old_value, new_value);
                listener->on_property_changed(sender, this);
            }

            PropertyBase::notify_change(sender);
        }

    private:

        const T m_default_value;

        std::vector<property_value_listener_ptr<T>> m_listeners;

        friend PropertyOwnerBase;
    };

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

        void notify_change(PropertyOwnerBase* sender, const T& value, bool add_or_remove) {
            for (auto& listener : m_listeners) {
                listener->on_collection_changed(sender, this, value, add_or_remove);
            }

            PropertyBase::notify_change(sender);
        }

    private:

        std::vector<collection_listener_ptr<T>> m_listeners;

        friend PropertyOwnerBase;
    };

    template <typename C>
    collection_property_ptr<C> make_collection() {
        return std::make_shared<ObservableCollectionProperty<C>>();
    }

    // Property owner

    class PropertyOwnerBase {

    public:

        virtual ~PropertyOwnerBase() = default;

    protected:

        void add_listener(const owner_listener_ptr& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const owner_listener_ptr& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

        void notify_change(property_base_ptr property) {
            for (const auto& listener : m_listeners) {
                listener->on_property_changed(this, property);
            }

            property->notify_change(this);
        }

        template<typename P>
        void register_property(property_ptr<P> property, P& field) {
            m_properties[property] = &field;
            field = property->get_default_value();
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
        void register_collection(collection_property_ptr<C> property, std::vector<C>& field) {
            m_properties[property] = &field;
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