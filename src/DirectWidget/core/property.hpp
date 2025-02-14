#pragma once

// property.hpp: Mutable property definition
// Mutable property can be used to define a property on a PropertyOwner with change notification

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace DirectWidget {

    class PropertyOwnerBase;

    template <typename T>
    class Property;

    template <typename T>
    using property_ptr = std::shared_ptr<Property<T>>;

    template <typename T>
    using PropertyChangeCallback = std::function<void(PropertyOwnerBase* sender, const T& old_value, const T& new_value)>;

    template <typename T>
    class ObservableCollectionProperty;

    template <typename T>
    using collection_property_ptr = std::shared_ptr<ObservableCollectionProperty<T>>;

    template <typename T>
    using CollectionChangedCallback = std::function<void(PropertyOwnerBase* sender, ObservableCollectionProperty<T>* property, const T& value, bool added)>;

    class PropertyBase {

    public:
        virtual ~PropertyBase() = default;

    };

    template <typename T>
    class Property : public PropertyBase {

    public:

        Property(const T& default_value) : m_default_value(default_value) {}

        const T& get_default_value() const {
            return m_default_value;
        }

        void register_callback(PropertyChangeCallback<T> callback) {
            if (callback == nullptr)
                return;
            m_callbacks.push_back(callback);
        }

        void notify_change(PropertyOwnerBase* sender, const T& old_value, const T& new_value) {
            for (auto& callback : m_callbacks) {
                callback(sender, old_value, new_value);
            }
        }

    private:

        const T m_default_value;

        std::vector<PropertyChangeCallback<T>> m_callbacks;
    };

    template <typename P>
    property_ptr<P> make_property(const P& default_value) {
        return std::make_shared<Property<P>>(default_value);
    }

    template <typename T>
    class ObservableCollectionProperty : public PropertyBase {

    public:

        void register_callback(CollectionChangedCallback<T> callback) {
            if (callback == nullptr)
                return;
            m_callbacks.push_back(callback);
        }

        void notify_change(PropertyOwnerBase* sender, const T& value, bool added) {
            for (auto& callback : m_callbacks) {
                callback(sender, this, value, added);
            }
        }

    private:

        std::vector<CollectionChangedCallback<T>> m_callbacks;
    };

    template <typename C>
    collection_property_ptr<C> make_collection(const CollectionChangedCallback<C>& callback) {
        auto result = std::make_shared<ObservableCollectionProperty<C>>();
        result->register_callback(callback);
        return result;
    }

    class PropertyOwnerBase {

    public:

        virtual ~PropertyOwnerBase() = default;

    protected:

        template<typename P>
        void register_property(property_ptr<P> property, P& field) {
            m_properties[property] = &field;
            field = property->get_default_value();
        }

        template<typename P>
        void set_property(const property_ptr<P>& property, const P& value) {
            for (auto& [prop, ptr] : m_properties) {
                if (prop == property) {
                    auto old_value = *static_cast<P*>(ptr);
                    *static_cast<P*>(ptr) = value;
                    property->notify_change(this, old_value, value);
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

    };

    using property_owner_ptr = std::shared_ptr<PropertyOwnerBase>;
}