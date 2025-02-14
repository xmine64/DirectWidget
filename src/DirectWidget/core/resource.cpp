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
