#include "injector.hpp"

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>

#include <TlHelp32.h>

namespace
{
    std::optional<DWORD> get_thread_exit_code(const HANDLE thread)
    {
        DWORD code{};
        if (!GetExitCodeThread(thread, &code))
        {
            return std::nullopt;
        }

        return code;
    }

    std::optional<DWORD> get_process_exit_code(const HANDLE thread)
    {
        DWORD code{};
        if (!GetExitCodeProcess(thread, &code))
        {
            return std::nullopt;
        }

        return code;
    }

    bool is_thread_running(const HANDLE process)
    {
        const auto code = get_thread_exit_code(process);
        return code && *code == STILL_ACTIVE;
    }

    bool is_process_running(const HANDLE process)
    {
        const auto code = get_process_exit_code(process);
        return code && *code == STILL_ACTIVE;
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

    uint8_t* allocate_memory(const HANDLE process, const size_t size, const uint32_t permissions = PAGE_READWRITE)
    {
        return static_cast<uint8_t*>(VirtualAllocEx(process, nullptr, size, MEM_COMMIT, permissions));
    }

    bool free_memory(const HANDLE process, void* memory)
    {
        return VirtualFreeEx(process, memory, 0, MEM_RELEASE);
    }

    bool is_64_bit_process(const HANDLE process)
    {
        // TODO: Support various architectures
        (void)process;
#if defined(_WIN32) && !defined(_WIN64)
        return false;
#else
        BOOL res{};
        IsWow64Process(process, &res);
        return !res;
#endif
    }
}

bool injection::release_memory()
{
    if (!this->process_ || !this->memory_)
    {
        return false;
    }

    const auto p = std::exchange(this->process_, {});
    auto* memory = std::exchange(this->memory_, nullptr);

    return free_memory(p, memory);
}

bool injection::done() const
{
    return !this->thread_ || !is_thread_running(this->thread_);
}

bool injection::succeeded() const
{
    return this->thread_ && get_thread_exit_code(this->thread_).value_or(1) == 0;
}

bool injection::await() const
{
    if (!this->thread_)
    {
        return false;
    }

    WaitForSingleObject(this->thread_, INFINITE);

    return this->succeeded();
}

injection::~injection()
{
    (void)this->await();
    this->release_memory();
}

injection::injection(utils::nt::null_handle process, utils::nt::null_handle thread, void* memory)
    : process_(std::move(process)),
      thread_(std::move(thread)),
      memory_(memory)
{
}

injector::injector()
{
    // TODO: Load LoadLibraryW RVAs
}

injection injector::inject(const DWORD process_id, const std::filesystem::path& dll) const
{
    constexpr auto access =
        SYNCHRONIZE | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION;
    utils::nt::null_handle process = OpenProcess(access, FALSE, process_id);

    if (!process)
    {
        return {};
    }

    return this->inject(std::move(process), dll);
}

injection injector::inject(utils::nt::null_handle process, const std::filesystem::path& dll) const
{
    const auto kernel32 = get_process_module_base(process, "kernel32.dll");

    const auto dll_name = dll.wstring();
    auto* memory = allocate_memory(process, (dll_name.size() + 1) * 2);

    if (!memory)
    {
        return {};
    }

    auto releaser = utils::finally([&] {
        free_memory(process, memory); //
    });

    const auto rva = is_64_bit_process(process) ? this->load_lib_rva_64 : this->load_lib_rva_32;
    auto* start_address = reinterpret_cast<LPTHREAD_START_ROUTINE>(kernel32 + rva);

    utils::nt::null_handle thread = CreateRemoteThread(process, nullptr, 0, start_address, memory, 0, nullptr);
    if (!thread)
    {
        return {};
    }

    releaser.cancel();

    return {std::move(process), std::move(thread), memory};
}
