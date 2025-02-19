// resource.hpp: Manageable resource definition
// Representing resources as a manageable object let's you invalidate them on property changes

#pragma once

#include <memory>
#include <type_traits>
#include <vector>
#include <functional>

#include "foundation.hpp"
#include "property.hpp"

namespace DirectWidget {

    class ResourceBase;
    using resource_base_ptr = std::shared_ptr<ResourceBase>;

    class ResourceListenerBase {

    public:
        virtual void on_resource_initialized(ResourceBase* resource) {}
        virtual void on_resource_invalidated(ResourceBase* resource) {}
        virtual void on_resource_updated(ResourceBase* resource) {}
    };
    using resource_listener_ptr = std::shared_ptr<ResourceListenerBase>;

    class ResourceBase {

    public:
        virtual ~ResourceBase();

        void add_listener(const resource_listener_ptr& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const resource_listener_ptr& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

        void bind(resource_base_ptr resource);
        void bind(property_base_ptr property);
        void unbind(resource_base_ptr resource);
        void unbind(property_base_ptr property);

        virtual void initialize() = 0;
        virtual void discard() = 0;

        bool is_valid() const { return m_valid; }

    protected:
        virtual bool try_update(const property_base_ptr& property) { return false; }
        virtual bool try_update(const resource_base_ptr& resource) { return false; }

        void mark_valid() { if (m_valid == true) return; m_valid = true; notify_change(); }
        void mark_invalid() { if (m_valid == false) return; m_valid = false; notify_change(); }

    private:
        void notify_change() {
            for (auto& listener : m_listeners) {
                if (m_valid) {
                    listener->on_resource_initialized(this);
                }
                else {
                    listener->on_resource_invalidated(this);
                }
            }
        }

        void notify_update() {
            for (auto& listener : m_listeners) {
                listener->on_resource_updated(this);
            }
        }

        bool m_valid = false;

        std::vector<resource_listener_ptr> m_listeners;

        class PropertyBinding : public PropertyListenerBase {

        public:

            PropertyBinding(ResourceBase* resource, property_base_ptr property) : m_resource(resource), m_property(property) {}

            const property_base_ptr& property() const { return m_property; }

            void on_property_changed(sender_ptr sender, property_token property) override {
                if (property != m_property.get()) return;
                if (m_resource->is_valid() && m_resource->try_update(m_property)) { m_resource->notify_update(); return; }
                m_resource->discard();
            }

        private:

            ResourceBase* m_resource;
            property_base_ptr m_property;
        };

        class ResourceBinding : public ResourceListenerBase {

        public:

            ResourceBinding(ResourceBase* self, resource_base_ptr dependency) : m_self(self), m_dependency(dependency) {}

            const resource_base_ptr& dependency() const { return m_dependency; }

            void handle_change(ResourceBase* resource) {
                if (m_self->is_valid() == false) return;
                if (resource != m_dependency.get()) return;
                if (m_self->try_update(m_dependency)) { 
                    m_self->notify_update();
                    return;
                }
                m_self->discard();
            }

            void on_resource_initialized(ResourceBase* resource) override {
                handle_change(resource);
            }

            void on_resource_invalidated(ResourceBase* resource) override {
                handle_change(resource);
            }

            void on_resource_updated(ResourceBase* resource) override {
                handle_change(resource);
            }

        private:

            ResourceBase* m_self;
            resource_base_ptr m_dependency;

        };

        std::vector<std::shared_ptr<PropertyBinding>> m_property_bindings;
        std::vector<std::shared_ptr<ResourceBinding>> m_resource_bindings;
    };

    template <typename T>
    class TypedResourceBase : public ResourceBase {
    public:
        virtual ~TypedResourceBase() = default;

        const T& get() {
            if (is_valid() == false) {
                initialize();
            }

            return get_internal();
        }

    protected:
        virtual const T& get_internal() const = 0;
    };

    template <typename T>
    class Resource : public TypedResourceBase<T> {

    public:
        Resource(const std::function<T()>& initializer) : m_initializer(initializer) {}
        ~Resource() { discard(); }

        void initialize() override {
            if (this->is_valid() == true) return;

            m_resource = m_initializer();
            this->mark_valid();
        }

        void discard() override {
            m_resource = T();
            this->mark_invalid();
        }

    protected:

        const T& get_internal() const {
            return m_resource;
        }

    private:

        std::function<T()> m_initializer;

        T m_resource;
    };

    template <typename T>
    using resource_ptr = std::shared_ptr<TypedResourceBase<T>>;

    template <typename T>
    resource_ptr<T> make_resource(const std::function<T()>& initializer) {
        return std::make_shared<Resource<T>>(initializer);
    }

    template<typename T>
    using com_resource_ptr = std::shared_ptr<TypedResourceBase<com_ptr<T>>>;

    template <typename T>
    com_resource_ptr<T> make_resource(const std::function<com_ptr<T>()>& initializer) {
        return std::make_shared<Resource<com_ptr<T>>>(initializer);
    }

}