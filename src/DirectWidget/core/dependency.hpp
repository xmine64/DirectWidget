// dependency.hpp: Base class for change notifier objects that can be used as "dependency"

#pragma once

#include <vector>
#include <memory>

namespace DirectWidget {

    class ElementBase;
    class DependencyBase;

    enum class NotificationType {
        Updated,
        Recreated,
        Initialized,
        Invalidated,
        ValueChanged,
        ElementAdded,
        ElementRemoved,
    };

    class NotificationArgument {
    public:
        NotificationArgument(NotificationType type, DependencyBase* dependency) : m_type(type), m_dependency(dependency) {}

        NotificationType notification_type() const { return m_type; }
        DependencyBase* dependency() const { return m_dependency; }

    private:
        NotificationType m_type;
        DependencyBase* m_dependency;
    };

    class DependencyListenerBase {
    public:
        virtual ~DependencyListenerBase() = default;
        virtual void on_dependency_updated(const ElementBase* owner, const NotificationArgument& arg) = 0;
    };

    using listener_ptr = std::shared_ptr<DependencyListenerBase>;

    class DependencyBase {
    public:
        void add_listener(const listener_ptr& listener) {
            m_listeners.push_back(listener);
        }

        void remove_listener(const listener_ptr& listener) {
            m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener));
        }

        virtual void register_owner(const ElementBase* owner) {}
        virtual void remove_owner(const ElementBase* owner) {}

    protected:
        void notify_updated(const ElementBase* owner, const NotificationArgument& arg) {
            for (auto& listener : m_listeners) {
                listener->on_dependency_updated(owner, arg);
            }
        }

    private:
        std::vector<listener_ptr> m_listeners;
    };

    using dependency_ptr = std::shared_ptr<DependencyBase>;
}