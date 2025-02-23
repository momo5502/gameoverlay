#include "overlay.hpp"

#include <utils/win.hpp>

namespace
{
    std::unique_ptr<utils::object>& get_overlay()
    {
        static std::unique_ptr<utils::object> overlay{};
        return overlay;
    }

    DWORD WINAPI initialize_async(LPVOID)
    {
        get_overlay() = gameoverlay::create_overlay();
        return 0;
    }

    void initialize()
    {
        CreateThread(nullptr, 0, initialize_async, nullptr, 0, nullptr);
    }
}

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD callReason, LPVOID /*reserved*/)
{
    switch (callReason)
    {
    case DLL_PROCESS_ATTACH:
        initialize();
        break;

    default:
        break;
    }

    return TRUE;
}
