#include "overlay.hpp"

#include <utils/win.hpp>

#pragma warning(push)
#pragma warning(disable : 4100)

#include <include/base/cef_bind.h>
#include <include/base/cef_callback.h>
#include <include/cef_app.h>
#include <include/cef_base.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_command_line.h>
#include <include/cef_frame.h>
#include <include/cef_parser.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_stream_resource_handler.h>

#pragma warning(pop)

namespace
{
    std::unique_ptr<utils::object>& get_overlay()
    {
        static std::unique_ptr<utils::object> overlay{};
        return overlay;
    }

    DWORD WINAPI initialize_async(LPVOID)
    {
        CefGetExitCode();
        get_overlay() = gameoverlay::create_overlay();
        return 0;
    }

    void initialize()
    {
        CreateThread(nullptr, 0, initialize_async, nullptr, 0, nullptr);
    }
}

BOOL APIENTRY DllMain(HMODULE /*module*/, const DWORD call_reason, LPVOID /*reserved*/)
{
    if (call_reason == DLL_PROCESS_ATTACH)
    {
        initialize();
    }

    return TRUE;
}
