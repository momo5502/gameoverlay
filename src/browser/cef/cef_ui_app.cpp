#include "cef_ui_app.hpp"

namespace gameoverlay
{
    cef_ui_app::cef_ui_app() = default;

    void cef_ui_app::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefV8Context> context)
    {
    }

    void cef_ui_app::OnContextInitialized()
    {
    }

    void cef_ui_app::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context)
    {
    }

    void cef_ui_app::OnBeforeCommandLineProcessing(const CefString& /*process_type*/,
                                                   CefRefPtr<CefCommandLine> command_line)
    {
        command_line->AppendSwitch("enable-experimental-web-platform-features");
        command_line->AppendSwitch("enable-media-stream");
    }
}
