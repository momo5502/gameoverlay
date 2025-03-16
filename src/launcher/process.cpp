#include "process.hpp"

namespace process
{
    std::optional<uint32_t> get_process_exit_code(const HANDLE process)
    {
        DWORD code{};
        if (!GetExitCodeProcess(process, &code))
        {
            return std::nullopt;
        }

        return code;
    }

    std::optional<uint32_t> get_thread_exit_code(const HANDLE thread)
    {
        DWORD code{};
        if (!GetExitCodeThread(thread, &code))
        {
            return std::nullopt;
        }

        return code;
    }

    bool is_process_running(const HANDLE process)
    {
        const auto code = get_process_exit_code(process);
        return code && *code == STILL_ACTIVE;
    }

    bool is_thread_running(const HANDLE thread)
    {
        const auto code = get_thread_exit_code(thread);
        return code && *code == STILL_ACTIVE;
    }

    bool is_32_bit_process(const HANDLE process)
    {
        BOOL res{};
        IsWow64Process(process, &res);
        return res;
    }
}
