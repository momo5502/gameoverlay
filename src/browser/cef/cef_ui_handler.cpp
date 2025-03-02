#include "cef_ui_handler.hpp"

#include "cef_ui.hpp"

using namespace std::literals;

namespace gameoverlay
{
    cef_ui_handler::cef_ui_handler(cef_ui& ui, web_ui_handler& handler)
        : ui_(&ui),
          handler_(&handler)
    {
        this->thread_ = std::thread([this] {
            dimensions last_dim{};

            while (!this->stop_)
            {
                const auto dim = this->handler_->get_dimensions();

                if (last_dim != dim)
                {
                    last_dim = dim;

                    cef_ui::post_on_ui([this] {
                        this->trigger_resize(); //
                    });
                }

                std::this_thread::sleep_for(10ms);
            }
        });
    }

    cef_ui_handler::~cef_ui_handler()
    {
        this->stop_ = true;

        if (this->thread_.joinable())
        {
            this->thread_.join();
        }
        this->browser_ = nullptr;
    }

    void cef_ui_handler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
    {
        CEF_REQUIRE_UI_THREAD()
        assert(!this->browser_ && "Only one browser supported by one ui handler");

        if (!this->browser_)
        {
            this->browser_ = browser;
        }

        this->ui_->store_browser(std::move(browser));
    }

    void cef_ui_handler::OnBeforeClose(const CefRefPtr<CefBrowser> browser)
    {
        CEF_REQUIRE_UI_THREAD()

        if (this->browser_ && this->browser_->IsSame(browser))
        {
            this->browser_ = nullptr;
        }

        this->ui_->remove_browser(browser);
    }

    void cef_ui_handler::OnBeforeContextMenu(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/,
                                             CefRefPtr<CefContextMenuParams> /*params*/, CefRefPtr<CefMenuModel> model)
    {
        model->Clear();
    }

    bool cef_ui_handler::OnContextMenuCommand(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/,
                                              CefRefPtr<CefContextMenuParams> /*params*/, int /*command_id*/,
                                              CefContextMenuHandler::EventFlags /*event_flags*/)
    {
        return false;
    }

    void cef_ui_handler::OnContextMenuDismissed(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/)
    {
    }

    bool cef_ui_handler::RunContextMenu(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/,
                                        CefRefPtr<CefContextMenuParams> /*params*/, CefRefPtr<CefMenuModel> /*model*/,
                                        CefRefPtr<CefRunContextMenuCallback> /*callback*/)
    {
        return false;
    }

    void cef_ui_handler::trigger_repaint()
    {
        CEF_REQUIRE_UI_THREAD()

        if (this->browser_)
        {
            this->browser_->GetHost()->Invalidate(PET_VIEW);
        }
    }

    void cef_ui_handler::trigger_resize()
    {
        CEF_REQUIRE_UI_THREAD()

        if (!this->browser_)
        {
            return;
        }

        this->browser_->GetHost()->WasResized();

        cef_ui::post_delayed_on_ui(
            [this] {
                this->trigger_repaint(); //
            },
            100ms);
    }

    void cef_ui_handler::GetViewRect(CefRefPtr<CefBrowser> /*browser*/, CefRect& rect)
    {
        const auto dimensions = this->handler_->get_dimensions();

        if (dimensions.is_non_zero())
        {
            rect.Set(0, 0, static_cast<int>(dimensions.width), static_cast<int>(dimensions.height));
            return;
        }

        rect.Set(0, 0, 640, 480);
    }

    void cef_ui_handler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType /*type*/,
                                 const RectList& /*dirty_rects*/, const void* buffer, const int width, const int height)
    {
        if (width < 0 || height < 0)
        {
            return;
        }

        const dimensions dim(width, height);
        const auto target_dim = this->handler_->get_dimensions();

        if (dim == target_dim)
        {
            const std::span data(static_cast<const uint8_t*>(buffer), static_cast<size_t>(4) * width * height);
            this->handler_->paint(data, dim);
        }
        else
        {
            browser->GetHost()->WasResized();
        }
    }
}
