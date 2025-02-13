// foundation.hpp: Includes basic definitions and types

#pragma once

#include <comdef.h>

template <typename Interface>
using com_ptr = _com_ptr_t<_com_IIID<Interface, &__uuidof(Interface)>>;

namespace DirectWidget {

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


    inline SIZE_F size_min(const SIZE_F& a, const SIZE_F& b) {
        return { a.width < b.width ? a.width : b.width, a.height < b.height ? a.height : b.height };
    }

}