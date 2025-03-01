#pragma once

#include <cef_include.hpp>

namespace gameoverlay
{
    class cef_ui_app : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler
    {
      public:
        cef_ui_app();

        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
        {
            return this;
        }

        CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
        {
            return this;
        }

        void OnContextInitialized() override;
        void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefV8Context> context) override;

      protected:
        void OnBeforeCommandLineProcessing(const CefString& process_type,
                                           CefRefPtr<CefCommandLine> command_line) override;
        void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefV8Context> context) override;

      private:
        IMPLEMENT_REFCOUNTING(cef_ui_app);
    };
}
