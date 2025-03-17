#include "injector.hpp"

#include <span>
#include <utils/nt.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>

#include "pe.hpp"
#include "process.hpp"
#include "snapshot.hpp"

namespace
{
    uint8_t* get_process_module_base(const HANDLE process, std::string module_name)
    {
        module_name = utils::string::to_lower(module_name);

        const auto modules = snapshot::get_modules(process);

        for (const auto& mod : modules)
        {
            if (utils::string::to_lower(mod.szModule) == module_name)
            {
                return mod.modBaseAddr;
            }
        }

        return nullptr;
    }

    uint8_t* try_get_process_module_base(const HANDLE process, const std::string& module_name)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            auto* base = get_process_module_base(process, module_name);
            if (base)
            {
                return base;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

bool injection::wait_for_thread(const bool no_wait)
{
    if (!this->thread_)
    {
        return true;
    }

    if (WaitForSingleObject(this->thread_, no_wait ? 0 : INFINITE) != WAIT_OBJECT_0)
    {
        return false;
    }

    this->release_memory();
    return true;
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
    // TODO: Handle 32 on 32
    this->load_lib_rva_64 = pe::find_export_rva(R"(C:\Windows\System32\kernel32.dll)", "LoadLibraryW");
    this->load_lib_rva_32 = pe::find_export_rva(R"(C:\Windows\SysWOW64\kernel32.dll)", "LoadLibraryW");
}

injection injector::inject(const DWORD process_id, const std::filesystem::path& dll) const
{
    utils::nt::null_handle process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);

    if (!process)
    {
        return {};
    }

    return this->inject(std::move(process), dll);
}

injection injector::inject(utils::nt::null_handle process, const std::filesystem::path& dll) const
{
    const auto kernel32 = try_get_process_module_base(process, "kernel32.dll");
    if (!kernel32)
    {
        return {};
    }

    const auto dll_name = dll.wstring();
    const auto dll_name_size = (dll_name.size() + 1) * 2;
    auto* memory = allocate_memory(process, dll_name_size);

    if (!memory)
    {
        return {};
    }

    auto releaser = utils::finally([&] {
        free_memory(process, memory); //
    });

    SIZE_T written{};
    const auto res = WriteProcessMemory(process, memory, dll_name.c_str(), dll_name_size, &written);
    if (!res || written != dll_name_size)
    {
        return {};
    }

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
