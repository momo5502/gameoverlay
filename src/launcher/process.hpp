#pragma once

#include <cstdint>
#include <optional>
#include <utils/win.hpp>

namespace process
{
    std::optional<uint32_t> get_process_exit_code(HANDLE process);
    std::optional<uint32_t> get_thread_exit_code(HANDLE thread);

    bool is_process_running(HANDLE process);
    bool is_thread_running(HANDLE thread);

    bool is_32_bit_process(HANDLE process);
}
