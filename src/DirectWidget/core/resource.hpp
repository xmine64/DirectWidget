// resource.hpp: Manageable resource definition
// Representing resources as a manageable object let's you invalidate them on property changes

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "foundation.hpp"
#include "dependency.hpp"

namespace DirectWidget {

    class ElementBase;
    class ResourceBase;

    class ResourceState {
    public:
        bool is_valid() const { return m_valid; }
        void set_valid(bool valid) { m_valid = valid; }
    private:
        bool m_valid = false;
    };

    class ResourceBase : public DependencyBase {
    public:
        virtual ~ResourceBase() = default;

        virtual void register_owner(const ElementBase* owner) override {
            m_state[owner] = ResourceState{};
        }

        virtual void remove_owner(const ElementBase* owner) override {
            invalidate_for(owner);
            m_state.erase(owner);
        }

        bool is_valid(const ElementBase* owner) const {
            auto it = m_state.find(owner);
            if (it != m_state.end()) {
                return it->second.is_valid();
            }
            return false;
        }

        void initialize_for(const ElementBase* owner) {
            if (initialize(owner)) {
                mark_valid(owner);
            }
        }

        void invalidate_for(const ElementBase* owner) {
            discard(owner);
            mark_invalid(owner);
        }

    protected:
        static const LogContext Logger;

        virtual bool initialize(const ElementBase* owner) = 0;
        virtual void discard(const ElementBase* owner) = 0;

        void mark_valid(const ElementBase* owner) {
            m_state[owner].set_valid(true);
            notify_initialization(owner);
        }

        virtual void mark_invalid(const ElementBase* owner) {
            m_state[owner].set_valid(false);
            notify_invalidation(owner);
        }

        void notify_initialization(const ElementBase* owner) {
            DependencyBase::notify_updated(owner, NotificationArgument(NotificationType::Initialized, this));
        }

        void notify_invalidation(const ElementBase* owner) {
            DependencyBase::notify_updated(owner, NotificationArgument(NotificationType::Invalidated, this));
        }

        void notify_updated(const ElementBase* owner) {
            DependencyBase::notify_updated(owner, NotificationArgument(NotificationType::Updated, this));
        }

    private:
        std::unordered_map<const ElementBase*, ResourceState> m_state;
    };

    using resource_base_ptr = std::shared_ptr<ResourceBase>;

    template <typename T>
    class Resource : virtual public ResourceBase {
    public:
        const T& get_or_initialize_resource(const ElementBase* owner) {
            if (is_valid(owner) == false) {
                initialize_for(owner);
            }
            return get_resource(owner);
        }

        virtual const T& get_resource(const ElementBase* owner) const = 0;
    };

    template <typename T>
    using resource_ptr = std::shared_ptr<Resource<T>>;

    class InheritedResourceBase : virtual public ResourceBase {
    public:
        virtual void register_owner(const ElementBase* owner) override {
            ResourceBase::register_owner(owner);
            m_parent[owner] = nullptr;
        }

        virtual void remove_owner(const ElementBase* owner) override {
            ResourceBase::remove_owner(owner);
            m_parent.erase(owner);
        }

        void register_parent(const ElementBase* owner, ElementBase* parent) {
            auto grandparent = m_parent[parent];
            if (grandparent == nullptr) {
                m_parent[owner] = parent;
            }
            else {
                m_parent[owner] = grandparent;
            }
        }

        void remove_parent(const ElementBase* owner) {
            m_parent[owner] = nullptr;
        }

    protected:
        ElementBase* get_parent_for(const ElementBase* owner) const {
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
    class InheritedResource : public InheritedResourceBase, public Resource<T> {
    public:
        InheritedResource(const resource_ptr<T>& source) : m_source(source) {}

        void register_owner(const ElementBase* owner) override {
            InheritedResourceBase::register_owner(owner);
        }

        void remove_owner(const ElementBase* owner) override {
            InheritedResourceBase::register_owner(owner);
        }

        const T& get_resource(const ElementBase* owner) const override {
            return m_source->get_resource(get_parent_for(owner));
        }

    protected:
        bool initialize(const ElementBase* owner) override {
            auto parent = get_parent_for(owner);
            m_source->initialize_for(parent);
            return m_source->is_valid(parent);
        }

        void discard(const ElementBase* owner) override {
            auto parent = get_parent_for(owner);
            if (parent != nullptr) {
                discard(parent);
            }
        }

    private:
        const resource_ptr<T>& m_source;
    };

    template <typename T>
    class BasicTypeResource : public Resource<T> {
    public:
        void register_owner(const ElementBase* owner) override {
            ResourceBase::register_owner(owner);
            m_resources[owner] = T();
        }

        void remove_owner(const ElementBase* owner) override {
            ResourceBase::remove_owner(owner);
            m_resources.erase(owner);
        }

        const T& get_resource(const ElementBase* owner) const override {
            auto it = m_resources.find(owner);
            if (it == m_resources.end()) {
                return T();
            }
            return it->second;
        }

        void update_resource(const ElementBase* owner, const T& value) {
            auto& resource = m_resources[owner];
            if (resource == value) return;
            m_resources[owner] = value;
            ResourceBase::notify_updated(owner);
        }

    protected:
        virtual bool initialize(const ElementBase* owner, T& resource) = 0;
        virtual void discard(const ElementBase* owner, T& resource) {}

        bool initialize(const ElementBase* owner) override {
            auto& resource = m_resources[owner];
            return initialize(owner, resource);
        }

        void discard(const ElementBase* owner) override {
            auto& resource = m_resources[owner];
            discard(owner, resource);
            resource = T();
        }

    private:
        std::unordered_map<const ElementBase*, T> m_resources;
    };

    template <typename T>
    class SharedResource : public Resource<std::shared_ptr<T>> {
    public:
        void register_owner(const ElementBase* owner) override {
            ResourceBase::register_owner(owner);
            m_resources[owner] = nullptr;
        }

        void remove_owner(const ElementBase* owner) override {
            ResourceBase::remove_owner(owner);
            m_resources.erase(owner);
        }

        const std::shared_ptr<T>& get_resource(const ElementBase* owner) const {
            return m_resources[owner];
        }

    protected:
        virtual bool initialize(const ElementBase* owner, std::shared_ptr<T>& resource) = 0;
        virtual void discard(const ElementBase* owner, std::shared_ptr<T>& resource) {}

        bool initialize(const ElementBase* owner) override {
            auto& resource = m_resources[owner];
            return initialize(owner, resource);
        }

        void discard(const ElementBase* owner) override {
            auto& resource = m_resources[owner];
            discard(owner, resource);
            resource = nullptr;
        }

    private:
        std::unordered_map<const ElementBase*, std::shared_ptr<T>> m_resources;
    };
}