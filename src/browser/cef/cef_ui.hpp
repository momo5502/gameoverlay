#pragma once

#include <cef_include.hpp>

#include <thread>
#include <filesystem>

#include <web_ui_handler.hpp>

namespace gameoverlay
{
    class cef_ui
    {
      public:
        cef_ui();
        ~cef_ui();

        static int run_process();

        static void post_on_ui(std::function<void()> callback);
        static void post_delayed_on_ui(std::function<void()> callback, std::chrono::milliseconds ms);

        void remove_browser(const CefRefPtr<CefBrowser>& browser);
        void store_browser(CefRefPtr<CefBrowser> browser);

        void create_browser(web_ui_handler& handler, std::string url);

      private:
        std::list<CefRefPtr<CefBrowser>> browsers_{};
        std::thread ui_thread_;

        void ui_runner();
        void close_all_browsers();
    };
}
