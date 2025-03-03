// property.hpp: Mutable property definition
// Mutable property can be used to define a property on a PropertyOwner with change notification

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "foundation.hpp"
#include "dependency.hpp"

namespace DirectWidget {

    class PropertyBase;
    using property_base_ptr = std::shared_ptr<PropertyBase>;
    using property_token = PropertyBase*;

    class ElementBase;

    template <typename T>
    class ValueChangeNotificationArgument : public NotificationArgument {
    public:
        ValueChangeNotificationArgument(DependencyBase* dependency, const T& old_value, const T& new_value)
            : NotificationArgument(NotificationType::ValueChanged, dependency), m_old_value(old_value), m_new_value(new_value) {
        }
        const T& old_value() const { return m_old_value; }
        const T& new_value() const { return m_new_value; }
    private:
        T m_old_value;
        T m_new_value;
    };

    template <typename T>
    class CollectionUpdateNotificationArgument : public NotificationArgument {
    public:
        CollectionUpdateNotificationArgument(DependencyBase* dependency, const T& value, bool add_or_remove)
            : NotificationArgument(
                add_or_remove 
                ? NotificationType::ElementAdded
                : NotificationType::ElementRemoved,
                dependency), m_value(value) {
        }
        const T& value() const { return m_value; }
    private:
        const T& m_value;
    };

    // Properties

    class PropertyBase : public DependencyBase {
    public:
        virtual ~PropertyBase() = default;
    };

    template <typename T>
    class TypedPropertyBase : virtual public PropertyBase {
    public:
        virtual ~TypedPropertyBase() = default;

        virtual const T& get_value(const ElementBase* owner) const = 0;
        virtual void set_value(const ElementBase* owner, const T& value) = 0;

    protected:
        void notify_change(const ElementBase* sender, const T& old_value, const T& new_value) {
            DependencyBase::notify_updated(sender, ValueChangeNotificationArgument<T>(this, old_value, new_value));
        }
    };

    template <typename T>
    using property_ptr = std::shared_ptr<TypedPropertyBase<T>>;

    class InheritedPropertyBase : virtual public PropertyBase {
    public:
        virtual ~InheritedPropertyBase() = default;

        void register_owner(const ElementBase* owner) override {
            m_parent[owner] = nullptr;
        }

        void remove_owner(const ElementBase* owner) override {
            m_parent.erase(owner);
        }

        void register_parent(const ElementBase* owner, ElementBase* parent) {
            m_parent[owner] = parent;
        }

        void remove_parent(const ElementBase* owner) {
            m_parent[owner] = nullptr;
        }

    protected:
        ElementBase* get_parent(const ElementBase* owner) const {
            auto it = m_parent.find(owner);
            if (it == m_parent.end()) {
                return nullptr;
            }
            return it->second;
        }

    private:
        std::unordered_map<const ElementBase*, ElementBase*> m_parent;
    };

    template <typename T>
    class InheritedProperty : public InheritedPropertyBase, public TypedPropertyBase<T> {
    public:
        InheritedProperty(const property_ptr<T>& source) : m_source(source) {}

        const T& get_value(const ElementBase* owner) const override {
            return m_source->get_value(get_parent(owner));
        }

        void set_value(const ElementBase* owner, const T& value) override {
            // TODO?
        }

    private:
        property_ptr<T> m_source;
    };

    template <typename T>
    class Property : public TypedPropertyBase<T> {
    public:
        Property(const T& default_value) : m_default_value(default_value) {}

        const T& get_default_value() const {
            return m_default_value;
        }

        void register_owner(const ElementBase* owner) override {
            m_values.insert({ owner, m_default_value });
        }

        void remove_owner(const ElementBase* owner) override {
            m_values.erase(owner);
        }

        const T& get_value(const ElementBase* owner) const override {
            auto iter = m_values.find(owner);
            if (iter != m_values.end()) {
                return iter->second;
            }
            return m_default_value;
        }

        void set_value(const ElementBase* owner, const T& value) override {
            auto old_value = m_values[owner];

            // FIXME:
            //if (old_value == value) return;

            m_values[owner] = value;
            TypedPropertyBase<T>::notify_change(owner, old_value, value);
        }

    private:
        const T m_default_value;
        std::unordered_map<const ElementBase*, T> m_values;
    };

    // Observable collection

    template <typename T>
    class ObservableCollectionProperty : public PropertyBase {
    public:
        void register_owner(const ElementBase* owner) override {
            m_values.insert({ owner, std::vector<T>() });
        }

        void remove_owner(const ElementBase* owner) override {
            m_values.erase(owner);
        }

        const std::vector<T>& get_values(const ElementBase* owner) const {
            auto iter = m_values.find(owner);
            if (iter == m_values.end()) {
                return std::vector<T>{};
            }
            return iter->second;
        }

        void add_element(ElementBase* owner, const T& element) {
            m_values[owner].push_back(element);
            notify_change(owner, element, true);
        }

        void remove_element(ElementBase* owner, const T& element) {
            auto& collection = m_values[owner];
            collection.erase(std::remove(collection.begin(), collection.end(), element));
            notify_change(owner, element, false);
        }

    protected:
        void notify_change(ElementBase* sender, const T& value, bool add_or_remove) {
            DependencyBase::notify_updated(sender, CollectionUpdateNotificationArgument<T>(this, value, add_or_remove));
        }

    private:
        std::unordered_map<const ElementBase*, std::vector<T>> m_values;
    };

    template <typename T>
    using collection_property_ptr = std::shared_ptr<ObservableCollectionProperty<T>>;

    // helper functions for making properties

    template <typename P>
    inline property_ptr<P> make_property(const P& default_value) {
        return std::make_shared<Property<P>>(default_value);
    }

    template <typename C>
    collection_property_ptr<C> make_collection() {
        return std::make_shared<ObservableCollectionProperty<C>>();
    }
}