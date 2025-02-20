// foundation.hpp: Includes basic definitions and types

#pragma once

#include <string>
#include <format>
#include <memory>

#include <Windows.h>
#include <comdef.h>

template <typename Interface>
using com_ptr = _com_ptr_t<_com_IIID<Interface, &__uuidof(Interface)>>;

#define NAMEOF(x) L#x

namespace DirectWidget {

    class LogContext {
    public:
        LogContext(const std::wstring& context) : m_prefix(context) {}
        LogContext(PCWSTR context) : m_prefix(context) {}

        LogContext at(PCWSTR context) const { return LogContext(std::format(L"{}: {}", m_prefix, context)); }

        void log(PCWSTR message) const;
        void log_error(PCWSTR message) const;
        void log_error(HRESULT hr) const;

        void fatal_exit(PCWSTR message) const;
        void fatal_exit(HRESULT hr) const;

    private:
        std::wstring m_prefix;
    };

    typedef struct {
        float x, y;
    } POINT_F;

    typedef struct {

        float x, y, width, height;

    } RECT_F;

    typedef struct {

        float left, top, right, bottom;

    } BOUNDS_F;

    typedef struct {
        float width, height;
    } SIZE_F;

    // ElementBase (Property-Resource container) class

    class ElementBase;
    using element_ptr = std::shared_ptr<ElementBase>;

    // Owner listener

    class PropertyOwnerChangeListenerBase;
    using owner_listener_ptr = std::shared_ptr<PropertyOwnerChangeListenerBase>;

    // Properties
    
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

    // Property listeners

    class PropertyListenerBase;
    using property_listener_ptr = std::shared_ptr<PropertyListenerBase>;

    template <typename T>
    class PropertyValueListener;
    template <typename T>
    using property_value_listener_ptr = std::shared_ptr<PropertyValueListener<T>>;

    template <typename T>
    class CollectionListener;
    template <typename T>
    using collection_listener_ptr = std::shared_ptr<CollectionListener<T>>;

    // Resources

    class ResourceBase;
    using resource_base_ptr = std::shared_ptr<ResourceBase>;

    template <typename T>
    class TypedResourceBase;
    template <typename T>
    using resource_ptr = std::shared_ptr<TypedResourceBase<T>>;

    template<typename T>
    using com_resource_ptr = std::shared_ptr<TypedResourceBase<com_ptr<T>>>;

    // Resource listeners

    class ResourceListenerBase;
    using resource_listener_ptr = std::shared_ptr<ResourceListenerBase>;
}