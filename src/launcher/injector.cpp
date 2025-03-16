#include "injector.hpp"

#include <iostream>
#include <utils/nt.hpp>

#include <TlHelp32.h>

#include "utils/string.hpp"

namespace
{
    bool is_process_running(const HANDLE process)
    {
        DWORD code{};
        if (!GetExitCodeProcess(process, &code))
        {
            return false;
        }

        return code == STILL_ACTIVE;
    }

    utils::nt::ihv_handle create_process_snapshot(const HANDLE process)
    {
        const auto process_id = GetProcessId(process);
        utils::nt::ihv_handle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);

        if (snapshot && is_process_running(process))
        {
            return snapshot;
        }

        return {};
    }

    std::list<MODULEENTRY32> get_process_modules(const HANDLE process)
    {
        const auto snapshot = create_process_snapshot(process);
        if (!snapshot)
        {
            return {};
        }

        std::list<MODULEENTRY32> modules{};

        MODULEENTRY32 module_entry{};
        module_entry.dwSize = sizeof(MODULEENTRY32);

        if (!Module32First(snapshot, &module_entry))
        {
            return {};
        }

        do
        {
            modules.push_back(module_entry);
        } while (Module32Next(snapshot, &module_entry));

        return modules;
    }

    uint8_t* get_process_module_base(const HANDLE process, std::string module_name)
    {
        module_name = utils::string::to_lower(module_name);

        const auto modules = get_process_modules(process);

        for (const auto& mod : modules)
        {
            if (utils::string::to_lower(mod.szModule) == module_name)
            {
                return mod.modBaseAddr;
            }
        }

        return nullptr;
    }
}

injector::injector()
{
}

bool injector::inject(DWORD process_id, const std::filesystem::path& dll)
{
    constexpr auto access =
        SYNCHRONIZE | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION;
    const utils::nt::null_handle process = OpenProcess(access, FALSE, process_id);

    if (!process)
    {
        return false;
    }

    this->inject(process, dll);
}

bool injector::inject(HANDLE process, const std::filesystem::path& dll)
{
    return false;
}
