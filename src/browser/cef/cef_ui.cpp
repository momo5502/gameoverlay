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

    cef_ui::cef_ui()
    {
        cef_loader::load_cef();

        this->ui_thread_ = std::thread([this] {
            this->ui_runner(); //
        });
    }

    cef_ui::~cef_ui()
    {
        this->close_all_browsers();

        if (this->ui_thread_.joinable())
        {
            this->ui_thread_.join();
        }
    }

    int cef_ui::run_process()
    {
        cef_loader::load_cef();
        return CefExecuteProcess(get_cef_main_args(), new cef_ui_app(), nullptr);
    }

    void cef_ui::ui_runner()
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

            CefRunMessageLoop();

            if (!this->browsers_.empty())
            {
                assert(false && "All browsers should have been destroyed!");
                this->browsers_.clear();
            }
        }
        CefShutdown();
    }

    void cef_ui::close_all_browsers()
    {
        if (!CefCurrentlyOn(TID_UI))
        {
            post_on_ui([this] {
                this->close_all_browsers(); //
            });

            return;
        }

        if (this->browsers_.empty())
        {
            CefQuitMessageLoop();
            return;
        }

        const auto browsers = this->browsers_;

        for (const auto& browser : browsers)
        {
            browser->GetHost()->CloseBrowser(true);
        }
    }

    void cef_ui::remove_browser(const CefRefPtr<CefBrowser>& browser)
    {
        CEF_REQUIRE_UI_THREAD()

        for (auto i = this->browsers_.begin(); i != this->browsers_.end();)
        {
            if ((*i)->IsSame(browser))
            {
                i = this->browsers_.erase(i);
            }
            else
            {
                ++i;
            }
        }

        if (this->browsers_.empty())
        {
            CefQuitMessageLoop();
        }
    }

    void cef_ui::store_browser(CefRefPtr<CefBrowser> browser)
    {
        CEF_REQUIRE_UI_THREAD()
        this->browsers_.push_back(std::move(browser));
    }

    void cef_ui::create_browser(web_ui_handler& handler, std::string url)
    {
        if (!CefCurrentlyOn(TID_UI))
        {
            post_on_ui([this, &handler, u = std::move(url)] {
                this->create_browser(handler, std::move(u)); //
            });

            return;
        }

        CefBrowserSettings browser_settings;
        browser_settings.windowless_frame_rate = 60;

        CefWindowInfo window_info{};
        window_info.SetAsWindowless(GetDesktopWindow());
        window_info.windowless_rendering_enabled = TRUE;

        CefBrowserHost::CreateBrowser(window_info, new cef_ui_handler(*this, handler), url, browser_settings, nullptr,
                                      nullptr);
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
}
