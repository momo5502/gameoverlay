#pragma once

#include <utils/nt.hpp>
#include <TlHelp32.h>

namespace snapshot
{
    utils::nt::ihv_handle create_snapshot(uint32_t flags, uint32_t process_id = 0);
    utils::nt::ihv_handle create_process_snapshot(HANDLE process, const uint32_t flags);

    std::list<PROCESSENTRY32> get_processes();
    std::list<MODULEENTRY32> get_modules(HANDLE process);
}
