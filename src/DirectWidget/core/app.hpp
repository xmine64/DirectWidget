// app.h: Application definition
// Application singleton manages apps global resources

#pragma once

// Standard headers

#include <memory>

// Windows headers

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <comdef.h>

// Local headers

#include "foundation.hpp"
#include "window.hpp"

namespace DirectWidget {

    class Application
    {
    public:
        
        static const std::unique_ptr<Application>& instance()
        {
            static auto app = std::unique_ptr<Application>(new Application());
            return app;
        }

        ~Application() {
            m_d2d.Release();
            m_dwrite.Release();
            CoUninitialize();
        }

        HRESULT initialize();

        int run_message_loop(Window& main_window, int nCmdShow);

        const com_ptr<ID2D1Factory>& d2d() const { return m_d2d; }
        const com_ptr<IDWriteFactory>& dwrite() const { return m_dwrite; }

#ifdef _DEBUG

        void enable_debug() { m_is_debug = true; }
        bool is_debug() const { return m_is_debug; }

#endif // DEBUG

    private:

        Application() {}

        com_ptr<ID2D1Factory> m_d2d = nullptr;
        com_ptr<IDWriteFactory> m_dwrite = nullptr;

#ifdef _DEBUG
        bool m_is_debug = false;
#endif // DEBUG

    };

}