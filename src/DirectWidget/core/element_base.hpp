#pragma once

#include <memory>
#include <vector>

#include "foundation.hpp"
#include "dependency.hpp"
#include "property.hpp"
#include "resource.hpp"

namespace DirectWidget {

    class ElementBase;
    using element_ptr = std::shared_ptr<ElementBase>;

    class ElementBase {
    public:
        virtual ~ElementBase() {
            for (auto& dependency : m_dependencies) {
                dependency->remove_owner(this);
            }
        }

    protected:
        void register_dependency(const dependency_ptr& dependency) {
            dependency->register_owner(this);
            m_dependencies.push_back(dependency);
        }

        void register_child(const element_ptr& child) {
            child->m_parent = this;
            for (auto& dependency : child->m_dependencies) {
                auto inherited_property = dynamic_pointer_cast<InheritedPropertyBase>(dependency);
                if (inherited_property == nullptr) {
                    auto inherited_resource = dynamic_pointer_cast<InheritedResourceBase>(dependency);
                    if (inherited_resource == nullptr) continue;
                    inherited_resource->register_parent(child.get(), this);
                }
                else {
                    inherited_property->register_parent(child.get(), this);
                }
            }
            m_children.push_back(child);
        }

        void detach_child(const element_ptr& child) {
            child->m_parent = nullptr;
            for (auto& dependency : child->m_dependencies) {
                auto inherited_property = dynamic_pointer_cast<InheritedPropertyBase>(dependency);
                if (inherited_property == nullptr) {
                    auto inherited_resource = dynamic_pointer_cast<InheritedResourceBase>(dependency);
                    if (inherited_resource == nullptr) continue;
                    inherited_resource->remove_parent(child.get());
                }
                else {
                    inherited_property->remove_parent(child.get());
                }
            }
            m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
        }

        template <typename T>
        const T& get_property(const property_ptr<T>& property) const {
            return property->get_value(this);
        }

        template <typename T>
        void set_property(const property_ptr<T>& property, const T& value) {
            property->set_value(this, value);
        }

        template <typename T>
        void add_to_collection(const collection_property_ptr<T>& collection, const T& value) {
            collection->add_element(this, value);
        }

        template <typename T>
        void remove_from_collection(const collection_property_ptr<T>& collection, const T& value) {
            collection->remove_element(this, value);
        }

    private:
        ElementBase* m_parent = nullptr;
        std::vector<dependency_ptr> m_dependencies;
        std::vector<element_ptr> m_children;
    };
}