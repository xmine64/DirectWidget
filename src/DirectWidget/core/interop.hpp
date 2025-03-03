#pragma once

#include <memory>
#include <unordered_map>

#include <Windows.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "foundation.hpp"
#include "property.hpp"
#include "resource.hpp"

namespace DirectWidget {
    namespace Interop {
        template <typename Interface>
        using com_ptr = _com_ptr_t<_com_IIID<Interface, &__uuidof(Interface)>>;

        template <typename T>
        class ComResource : public Resource<com_ptr<T>> {
        public:
            void register_owner(const ElementBase* owner) override {
                ResourceBase::register_owner(owner);
                m_resources[owner] = nullptr;
            }

            void remove_owner(const ElementBase* owner) override {
                ResourceBase::remove_owner(owner);
                m_resources.erase(owner);
            }

            const com_ptr<T>& get_resource(const ElementBase* owner) const {
                auto it = m_resources.find(owner);
                if (it == m_resources.end()) {
                    return nullptr;
                }
                return it->second;
            }

        protected:
            virtual HRESULT initialize(const ElementBase* owner, com_ptr<T>& resource) = 0;
            virtual void discard(const ElementBase* owner, com_ptr<T>& resource) {}

            bool initialize(const ElementBase* owner) override {
                auto& resource = m_resources[owner];
                auto hr = initialize(owner, resource);
                ResourceBase::Logger.at(NAMEOF(ComResource<T>::initialize)).log_error(hr);
                return SUCCEEDED(hr);
            }

            void discard(const ElementBase* owner) override {
                auto& resource = m_resources[owner];
                discard(owner, resource);
                resource = nullptr;
            }

        private:
            std::unordered_map<const ElementBase*, com_ptr<T>> m_resources;
        };

        template<typename T>
        using com_resource_ptr = std::shared_ptr<ComResource<T>>;

        template<typename T>
        using inherited_com_resource_ptr = std::shared_ptr<InheritedResource<com_ptr<T>>>;

        class SolidColorBrushResource : public ComResource<ID2D1SolidColorBrush> {
        public:
            SolidColorBrushResource(const property_ptr<D2D1_COLOR_F>& color_property) : m_color_property(color_property) {}

        protected:
            HRESULT initialize(const ElementBase* owner, com_ptr<ID2D1SolidColorBrush>& resource) override;

        private:
            property_ptr<D2D1_COLOR_F> m_color_property;
        };

        inline D2D1_RECT_F to_d2d(const RECT_F& rect) {
            return D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
        }

        inline D2D1_RECT_F to_d2d(const BOUNDS_F& bounds) {
            return D2D1::RectF(bounds.left, bounds.top, bounds.right, bounds.bottom);
        }

        inline D2D1_SIZE_F to_d2d(const SIZE_F& size) {
            return D2D1::SizeF(size.width, size.height);
        }
    }
}