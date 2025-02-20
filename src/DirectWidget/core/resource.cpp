#include <memory>

#include "property.hpp"
#include "resource.hpp"

using namespace DirectWidget;

ResourceBase::~ResourceBase()
{
    for (auto& [element, binding] : m_property_bindings) {
        binding->property()->remove_listener(binding);
    }
    for (auto& binding : m_resource_bindings) {
        binding->dependency()->remove_listener(binding);
    }
}

void DirectWidget::ResourceBase::bind(resource_base_ptr resource)
{
    auto binding = std::make_shared<ResourceBinding>(this, resource);
    resource->add_listener(binding);
    m_resource_bindings.push_back(binding);
}

void ResourceBase::bind(element_ptr owner, property_base_ptr property)
{
    std::shared_ptr<PropertyBinding> binding;
    auto iter = m_property_bindings.find(property);
    if (iter == m_property_bindings.end()) {
        binding = std::make_shared<PropertyBinding>(this, property);
        property->add_listener(binding);
        m_property_bindings.insert({ property, binding });
    }
    else {
        binding = iter->second;
    }
    
    binding->register_owner(owner);
}

void ResourceBase::unbind(resource_base_ptr resource)
{
    auto iter = std::find_if(m_resource_bindings.begin(), m_resource_bindings.end(), [resource](const std::shared_ptr<ResourceBinding>& binding) {
        return binding->dependency() == resource;
        });
    if (iter == m_resource_bindings.end()) { return; }
    resource->remove_listener(*iter);
    m_resource_bindings.erase(iter);
}

void ResourceBase::unbind(element_ptr owner, property_base_ptr property)
{
    auto iter = m_property_bindings.find(property);
    if (iter == m_property_bindings.end()) return;
    iter->second->unregister_owner(owner);
}
