#pragma once

#include <cef_include.hpp>

#include "../browser_handler.hpp"

namespace gameoverlay
{
    class cef_ui_handler : public CefClient,
                           public CefDisplayHandler,
                           public CefLifeSpanHandler,
                           public CefRenderHandler,
                           public CefLoadHandler,
                           public CefContextMenuHandler
    {
      public:
        explicit cef_ui_handler(browser_handler& handler);
        ~cef_ui_handler() override;

        CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
        {
            return this;
        }

        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
        {
            return this;
        }

        CefRefPtr<CefRenderHandler> GetRenderHandler() override
        {
            return this;
        }

        CefRefPtr<CefLoadHandler> GetLoadHandler() override
        {
            return this;
        }

        CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override
        {
            return this;
        }

        void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirty_rects,
                     const void* buffer, int width, int height) override;

        void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
        void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

        void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;
        bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefContextMenuParams> params, int command_id,
                                  EventFlags event_flags) override;
        void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;
        bool RunContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model,
                            CefRefPtr<CefRunContextMenuCallback> callback) override;

        void close_all_browsers(bool force_close);
        void trigger_resize();

      private:
        browser_handler* handler_ = nullptr;
        std::vector<CefRefPtr<CefBrowser>> browser_list_{};

        IMPLEMENT_REFCOUNTING(cef_ui_handler);
    };
}
