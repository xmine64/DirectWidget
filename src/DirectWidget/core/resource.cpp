#include <memory>

#include "property.hpp"
#include "resource.hpp"

using namespace DirectWidget;

ResourceBase::~ResourceBase()
{
    for (auto& binding : m_property_bindings) {
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

void ResourceBase::bind(property_base_ptr property)
{
    auto binding = std::make_shared<PropertyBinding>(this, property);
    property->add_listener(binding);
    m_property_bindings.push_back(binding);
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

void ResourceBase::unbind(property_base_ptr property)
{
    auto iter = std::find_if(m_property_bindings.begin(), m_property_bindings.end(), [property](const std::shared_ptr<PropertyBinding>& binding) {
        return binding->property() == property;
        });
    if (iter == m_property_bindings.end()) { return; }
    property->remove_listener(*iter);
    m_property_bindings.erase(iter);
}
