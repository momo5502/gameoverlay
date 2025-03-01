#include "overlay.hpp"

#include <utils/win.hpp>
#include <utils/hook.hpp>

#include <cef_include.hpp>

namespace
{
    utils::hook::detour exit_hook{};

    std::unique_ptr<utils::object>& get_overlay()
    {
        static std::unique_ptr<utils::object> overlay{};
        return overlay;
    }

    void on_exit()
    {
        get_overlay() = {};
    }

    [[noreturn]] void NTAPI rtl_exit_user_process_stub(const DWORD exit_status)
    {
        auto* orig = exit_hook.get_place();
        exit_hook.clear();

        on_exit();

        static_cast<decltype(&rtl_exit_user_process_stub)>(orig)(exit_status);
    }

    void install_exit_hook()
    {
        const utils::nt::library ntdll("ntdll.dll");
        auto* exit_process = ntdll.get_proc<void*>("RtlExitUserProcess");
        exit_hook.create(exit_process, rtl_exit_user_process_stub);
    }

    DWORD WINAPI initialize_async(LPVOID)
    {
        install_exit_hook();
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
