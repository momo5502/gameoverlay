#include "cef_loader.hpp"

#include <utils/nt.hpp>

#include <delayimp.h>

namespace cef_loader
{
    namespace
    {
        std::filesystem::path get_own_directory()
        {
            const auto self = utils::nt::library::get_by_address(&get_own_directory);
            return self.get_folder();
        }

        void* load_cef_library()
        {
            const auto cef_path = get_cef_path();
            const auto cef_lib = cef_path / "libcef.dll";

            const auto cookie = AddDllDirectory(cef_path.wstring().c_str());
            const auto library = LoadLibraryExW(cef_lib.wstring().c_str(), nullptr,
                                                LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 |
                                                    LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

            RemoveDllDirectory(cookie);

            if (library)
            {
                return library;
            }

            throw std::runtime_error("Failed to load cef");
        }

        FARPROC WINAPI delay_load_hook(unsigned /*dliNotify*/, const PDelayLoadInfo pdli)
        {
            constexpr std::string_view cef = "libcef.dll";
            if (pdli->szDll != cef)
            {
                return nullptr;
            }

            auto* cef_dll = static_cast<HMODULE>(load_cef());
            auto* name = pdli->dlp.fImportByName //
                             ? pdli->dlp.szProcName
                             : MAKEINTRESOURCEA(pdli->dlp.dwOrdinal);

            return GetProcAddress(cef_dll, name);
        }
    }

    std::filesystem::path get_cef_path()
    {
        return get_own_directory() / "cef";
    }

    void* load_cef()
    {
        static const auto cef = load_cef_library();
        return cef;
    }
}

EXTERN_C
#ifndef DELAYIMP_INSECURE_WRITABLE_HOOKS
const
#endif
    PfnDliHook __pfnDliNotifyHook2 = cef_loader::delay_load_hook;
