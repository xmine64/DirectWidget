// DemoApp.cpp : Defines the entry point for the application.
//

#include <memory>
#include <type_traits>

#include <Windows.h>
#include <comdef.h>

#include "../DirectWidget/core/foundation.hpp"
#include "../DirectWidget/core/app.hpp"
#include "../DirectWidget/core/window.hpp"
#include "../DirectWidget/widgets/button_widget.hpp"
#include "../DirectWidget/widgets/text_widget.hpp"
#include "../DirectWidget/layouts/stack_layout.hpp"

using namespace std;
using namespace DirectWidget;
using namespace DirectWidget::Widgets;
using namespace DirectWidget::Layouts;

class MainWindow : public Window
{
public:
    MainWindow() {

        auto row = make_unique<StackLayout>();
        row->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        
        auto title = make_unique<TextWidget>();
        title->set_text(L"Demo App");
        title->set_margin(BOUNDS_F{ 8.0f, 8.0f, 8.0f, 8.0f });
        title->set_font_size(24.0f);
        title->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
        row->add_widget(move(title));

        auto description = make_unique<TextWidget>();
        description->set_text(L"This is a sample demo app using DirectWidget.");
        description->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        row->add_widget(move(description));

        auto counter = make_unique<TextWidget>();
        counter->set_text(L"Counter: 0");
        counter->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        row->add_widget(move(counter));

        auto center_align = make_unique<TextWidget>();
        center_align->set_text(L"Center Aligned");
        center_align->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        center_align->set_horizontal_alignment(WIDGET_ALIGNMENT_CENTER);
        row->add_widget(move(center_align));

        auto end_align = make_unique<TextWidget>();
        end_align->set_text(L"End Aligned");
        end_align->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        end_align->set_horizontal_alignment(WIDGET_ALIGNMENT_END);
        row->add_widget(move(end_align));

        auto buttons_column = make_unique<StackLayout>();
        buttons_column->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        buttons_column->set_horizontal();

        auto increment_button = make_unique<ButtonWidget>();
        increment_button->set_text(L"Increment");
        increment_button->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        increment_button->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
        increment_button->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        buttons_column->add_widget(std::move(increment_button));

        auto exit_button = make_unique<ButtonWidget>();
        exit_button->set_text(L"Exit");
        exit_button->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        exit_button->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
        exit_button->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        exit_button->set_click_handler([this]() {
            DestroyWindow(this->window_handle());
        });
        buttons_column->add_widget(move(exit_button));

        row->add_widget(move(buttons_column));

        set_root_widget(move(row));
    }

    PCWSTR title() const override { return L"Demo App"; }
    PCWSTR class_name() const override { return L"DemoApp_MainWindow"; }

    bool on_destroy() override { PostQuitMessage(0); return true; }

private:
    int m_counter = 0;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto& app = DirectWidget::Application::instance();
    
#ifdef _DEBUG
    //app->enable_debug();
#endif // DEBUG

    auto hr = app->initialize();
    if (FAILED(hr)) {
        _com_error err(hr);
        FatalAppExit(0, err.ErrorMessage());
        return 1;
    }

    MainWindow mainWindow;
    return app->run_message_loop(mainWindow, nCmdShow);
}
