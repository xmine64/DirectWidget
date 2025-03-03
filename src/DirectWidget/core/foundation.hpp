// foundation.hpp: Includes basic definitions and types

#pragma once

#include <string>
#include <format>
#include <memory>

#include <Windows.h>

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

    

    // Properties
    
    

    // Property listeners

    

    

    

    // Resources

    

    

    // Resource listeners

    
}