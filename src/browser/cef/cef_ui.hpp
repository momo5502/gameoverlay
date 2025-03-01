#pragma once

#include <cef_include.hpp>

#include <thread>
#include <filesystem>

#include "../browser_handler.hpp"

namespace gameoverlay
{
    class cef_ui_app;
    class cef_ui_handler;

    class cef_ui
    {
      public:
        cef_ui(browser_handler& handler);
        ~cef_ui();

        static int run_process();

        void close_browser();

        static void post_on_ui(std::function<void()> callback);
        static void post_delayed_on_ui(std::function<void()> callback, std::chrono::milliseconds ms);

      private:
        CefRefPtr<CefBrowser> browser_;

        std::thread ui_thread_;

        void ui_runner(browser_handler& handler);

        static void invoke_close_browser(const CefRefPtr<CefBrowser>& browser);
    };
}
