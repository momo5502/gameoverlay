#include "overlay.hpp"

#include <utils/win.hpp>

#include <cef_include.hpp>

namespace
{
    std::unique_ptr<utils::object>& get_overlay()
    {
        static std::unique_ptr<utils::object> overlay{};
        return overlay;
    }

    DWORD WINAPI initialize_async(LPVOID)
    {
        // CefGetExitCode();
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
