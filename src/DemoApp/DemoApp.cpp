// DemoApp.cpp : Defines the entry point for the application.
//

#include <memory>
#include <type_traits>
#include <string>

#include <Windows.h>
#include <comdef.h>

#include "../DirectWidget/core/foundation.hpp"
#include "../DirectWidget/core/app.hpp"
#include "../DirectWidget/core/window.hpp"
#include "../DirectWidget/core/widget.hpp"
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

        auto row = make_shared<StackLayout>();
        row->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        row->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        row->set_vertical_alignment(WIDGET_ALIGNMENT_STRETCH);
        row->set_orientation(STACK_LAYOUT_VERTICAL);
        
        auto title = make_shared<TextWidget>();
        title->set_text(L"Demo App");
        title->set_margin(BOUNDS_F{ 8.0f, 8.0f, 8.0f, 8.0f });
        title->set_font_size(24.0f);
        title->set_horizontal_alignment(WIDGET_ALIGNMENT_START);
        title->set_vertical_alignment(WIDGET_ALIGNMENT_START);
        row->add_child(title);

        auto description = make_shared<TextWidget>();
        description->set_text(L"This is a sample demo app using DirectWidget.");
        description->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        description->set_horizontal_alignment(WIDGET_ALIGNMENT_START);
        row->add_child(description);

        auto counter = make_shared<TextWidget>();
        counter->set_text(L"Counter: 0");
        counter->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        counter->set_horizontal_alignment(WIDGET_ALIGNMENT_START);
        row->add_child(counter);
        m_counter_widget = counter;

        auto center_align = make_shared<TextWidget>();
        center_align->set_text(L"Center Aligned");
        center_align->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        center_align->set_horizontal_alignment(WIDGET_ALIGNMENT_CENTER);
        row->add_child(center_align);

        auto end_align = make_shared<TextWidget>();
        end_align->set_text(L"End Aligned");
        end_align->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        end_align->set_horizontal_alignment(WIDGET_ALIGNMENT_END);
        row->add_child(end_align);

        auto stretched = make_shared<TextWidget>();
        stretched->set_text(L"Stretched widget with center text alignment");
        stretched->set_margin(BOUNDS_F{ 4.0f,4.0f,4.0f,4.0f });
        stretched->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        stretched->set_text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        row->add_child(stretched);

        auto buttons_column = make_shared<StackLayout>();
        buttons_column->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        buttons_column->set_orientation(STACK_LAYOUT_HORIZONTAL);

        auto update_button = make_shared<ButtonWidget>();
        update_button->set_text(L"set stretched text alignment to trailing");
        update_button->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        update_button->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
        update_button->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        update_button->set_click_handler([this, stretched]() {
            stretched->set_text_alignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
            });
        buttons_column->add_child(update_button);

        auto increment_button = make_shared<ButtonWidget>();
        increment_button->set_text(L"Increment");
        increment_button->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        increment_button->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
        increment_button->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        increment_button->set_click_handler([this]() {
            m_counter_text.clear();
            m_counter_text.append(L"Counter: ");
            m_counter_text.append(to_wstring(++m_counter));
            m_counter_widget->set_text(m_counter_text.c_str());
            });
        buttons_column->add_child(increment_button);

        auto exit_button = make_shared<ButtonWidget>();
        exit_button->set_text(L"Exit");
        exit_button->set_margin(BOUNDS_F{ 4.0f, 4.0f, 4.0f, 4.0f });
        exit_button->set_vertical_alignment(WIDGET_ALIGNMENT_CENTER);
        exit_button->set_horizontal_alignment(WIDGET_ALIGNMENT_STRETCH);
        exit_button->set_click_handler([this]() {
            DestroyWindow(this->window_handle());
        });
        buttons_column->add_child(exit_button);

        row->add_child(buttons_column);

        set_root_widget(row);
    }

    PCWSTR title() const override { return L"Demo App"; }
    PCWSTR class_name() const override { return L"DemoApp_MainWindow"; }

    bool on_destroy() override { PostQuitMessage(0); return true; }

private:

    int m_counter = 0;
    wstring m_counter_text{ L"Counter: 0" };
    shared_ptr<TextWidget> m_counter_widget;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LogContext log{ L"wWinMain" };

    auto& app = DirectWidget::Application::instance();
    
#ifdef _DEBUG
    app->enable_debug();
#endif // DEBUG

    auto hr = app->initialize();
    log.at(L"Application::initialize").fatal_exit(hr);

    MainWindow mainWindow;
    return app->run_message_loop(mainWindow, nCmdShow);
}
