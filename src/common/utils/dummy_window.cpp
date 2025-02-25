#include "dummy_window.hpp"
#include <atomic>

namespace utils
{
    namespace
    {

        std::string get_unique_window_class_name()
        {
            static std::atomic_uint64_t counter{0};
            return "dummy-window-" + std::to_string(GetCurrentProcessId()) + "-" +
                   std::to_string(GetCurrentThreadId()) + "-" + std::to_string(counter++);
        }

        void register_window_class(const std::string& class_name)
        {
            WNDCLASSA wc = {};
            wc.lpfnWndProc = DefWindowProcA;
            wc.hInstance = GetModuleHandleA(nullptr);
            wc.lpszClassName = class_name.c_str();

            RegisterClassA(&wc);
        }

        HWND create_dummy_window(const std::string& class_name)
        {
            const auto hwnd =
                CreateWindowExA(WS_EX_TOOLWINDOW, class_name.c_str(), "", WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);

            if (hwnd)
            {
                ShowWindow(hwnd, SW_HIDE);
                UpdateWindow(hwnd);
            }

            return hwnd;
        }
    }

    dummy_window::dummy_window()
        : window_class_(get_unique_window_class_name()),
          window_(create_dummy_window(window_class_))
    {
    }

    dummy_window::~dummy_window()
    {
        if (this->window_)
        {
            DestroyWindow(this->window_);
        }

        if (!this->window_class_.empty())
        {
            UnregisterClassA(this->window_class_.c_str(), GetModuleHandleA(nullptr));
        }
    }
}
