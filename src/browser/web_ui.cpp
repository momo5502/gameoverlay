#include <web_ui.hpp>

#include "cef/cef_ui.hpp"

namespace gameoverlay
{
    web_ui::web_ui()
        : ui_(std::make_unique<cef_ui>())
    {
    }

    web_ui::~web_ui() = default;

    void web_ui::create_browser(web_ui_handler& handler, std::string url)
    {
        this->ui_->create_browser(handler, std::move(url));
    }

    int web_ui::run_process()
    {
        return cef_ui::run_process();
    }
}
