#include "cef_ui.hpp"
#include "cef_ui_app.hpp"
#include "cef_ui_handler.hpp"

#include <cef_loader.hpp>

#include <utils/nt.hpp>

namespace gameoverlay
{
    namespace
    {
        std::filesystem::path get_own_directory()
        {
            const auto self = utils::nt::library::get_by_address(&get_own_directory);
            return self.get_folder();
        }

        CefMainArgs get_cef_main_args()
        {
            const utils::nt::library main{};
            return CefMainArgs(main.get_handle());
        }
    }

    int cef_ui::run_process()
    {
        cef_loader::load_cef();
        return CefExecuteProcess(get_cef_main_args(), new cef_ui_app(), nullptr);
    }

    void cef_ui::ui_runner(browser_handler& handler)
    {
        {
            CefSettings settings{};
            settings.no_sandbox = FALSE;
            settings.windowless_rendering_enabled = TRUE;
            settings.remote_debugging_port = 28920;

#ifdef DEBUG
            settings.log_severity = LOGSEVERITY_ERROR;

#else
            settings.log_severity = LOGSEVERITY_DISABLE;
#endif

            const auto own_dir = get_own_directory();
            const auto cef_path = cef_loader::get_cef_path();

            CefString(&settings.browser_subprocess_path) = own_dir / "browser-process.exe";
            CefString(&settings.locales_dir_path) = cef_path / "locales";
            CefString(&settings.resources_dir_path) = cef_path;
            CefString(&settings.log_file) = own_dir / "cef_data/debug.log";
            CefString(&settings.cache_path) = own_dir / "cef_data/cache";
            CefString(&settings.locale) = "en-US";

            CefInitialize(get_cef_main_args(), settings, new cef_ui_app(), nullptr);

            CefBrowserSettings browser_settings;
            browser_settings.windowless_frame_rate = 60;

            std::string url = "http://google.com";

            CefWindowInfo window_info{};
            window_info.SetAsWindowless(GetDesktopWindow());
            window_info.windowless_rendering_enabled = TRUE;

            this->browser_ = CefBrowserHost::CreateBrowserSync(window_info, new cef_ui_handler(handler), url,
                                                               browser_settings, nullptr, nullptr);

            CefRunMessageLoop();
            this->browser_ = nullptr;
        }
        CefShutdown();
    }

    void cef_ui::invoke_close_browser(const CefRefPtr<CefBrowser>& browser)
    {
        if (!browser)
        {
            return;
        }

        browser->GetHost()->CloseBrowser(true);
    }

    void cef_ui::close_browser()
    {
        if (!this->browser_)
        {
            return;
        }

        auto browser = this->browser_;
        this->browser_ = nullptr;

        post_on_ui([&, b = std::move(browser)] {
            invoke_close_browser(b); //
        });
    }

    void cef_ui::post_on_ui(std::function<void()> callback)
    {
        CefPostTask(TID_UI, base::BindOnce(
                                [](const std::function<void()>& c) {
                                    if (c)
                                    {
                                        c();
                                    }
                                },
                                std::move(callback)));
    }

    void cef_ui::post_delayed_on_ui(std::function<void()> callback, const std::chrono::milliseconds ms)
    {
        CefPostDelayedTask(TID_UI,
                           base::BindOnce(
                               [](const std::function<void()>& c) {
                                   if (c)
                                   {
                                       c();
                                   }
                               },
                               std::move(callback)),
                           ms.count());
    }

    cef_ui::cef_ui(browser_handler& handler)
    {
        cef_loader::load_cef();

        this->ui_thread_ = std::thread([this, &handler] {
            this->ui_runner(handler); //
        });
    }

    cef_ui::~cef_ui()
    {
        if (this->browser_)
        {
            this->close_browser();
        }

        if (this->ui_thread_.joinable())
        {
            this->ui_thread_.join();
        }
    }
}
