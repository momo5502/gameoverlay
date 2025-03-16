#include "snapshot.hpp"
#include "process.hpp"

namespace snapshot
{
    namespace
    {

        DWORD get_module_enum_flag(const HANDLE process)
        {
            (void)process;
#if defined(_WIN32) && !defined(_WIN64)
            return TH32CS_SNAPMODULE;
#else
            return process::is_32_bit_process(process) ? TH32CS_SNAPMODULE32 : TH32CS_SNAPMODULE;
#endif
        }
    }

    utils::nt::ihv_handle create_snapshot(const uint32_t flags, const uint32_t process_id)
    {
        return CreateToolhelp32Snapshot(flags, process_id);
    }

    utils::nt::ihv_handle create_process_snapshot(const HANDLE process, const uint32_t flags)
    {
        const auto process_id = GetProcessId(process);
        auto snapshot = create_snapshot(flags, process_id);

        if (snapshot && process::is_process_running(process))
        {
            return snapshot;
        }

        return {};
    }

    std::list<MODULEENTRY32> get_modules(const HANDLE process)
    {
        const auto flags = get_module_enum_flag(process);
        const auto snapshot = create_process_snapshot(process, flags);
        if (!snapshot)
        {
            return {};
        }

        std::list<MODULEENTRY32> entries{};

        MODULEENTRY32 entry{};
        entry.dwSize = sizeof(entry);

        if (!Module32First(snapshot, &entry))
        {
            return {};
        }

        do
        {
            entries.push_back(entry);
        } while (Module32Next(snapshot, &entry));

        return entries;
    }

    std::list<PROCESSENTRY32> get_processes()
    {
        const auto snapshot = create_snapshot(TH32CS_SNAPPROCESS);
        if (!snapshot)
        {
            return {};
        }

        std::list<PROCESSENTRY32> entries{};

        PROCESSENTRY32 entry{};
        entry.dwSize = sizeof(entry);

        if (!Process32First(snapshot, &entry))
        {
            return {};
        }

        do
        {
            entries.push_back(entry);
        } while (Process32Next(snapshot, &entry));

        return entries;
    }
}
