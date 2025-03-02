#pragma once

#include <memory>
#include <string>

#include "web_ui_handler.hpp"

namespace gameoverlay
{
    class cef_ui;

    class web_ui
    {
      public:
        web_ui();
        ~web_ui();

        void create_browser(web_ui_handler& handler, std::string url);

        static int run_process();

      private:
        std::unique_ptr<cef_ui> ui_{};
    };
}
