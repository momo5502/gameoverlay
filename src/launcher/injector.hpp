#pragma once

#include <utils/nt.hpp>
#include <utils/class_helper.hpp>

class injector;

class injection
{
  public:
    friend injector;

    CLASS_DISABLE_COPY(injection);

    ~injection();

    injection(injection&&) noexcept = default;
    injection& operator=(injection&&) noexcept = default;

    operator bool();

    bool done() const;
    bool succeeded() const;

    bool await() const;

  private:
    utils::nt::null_handle process_{};
    utils::nt::null_handle thread_{};
    void* memory_{};

    injection() = default;
    injection(utils::nt::null_handle process, utils::nt::null_handle thread, void* memory);

    bool release_memory();
};

class injector
{
  public:
    injector();

    injection inject(DWORD process_id, const std::filesystem::path& dll) const;
    injection inject(utils::nt::null_handle process, const std::filesystem::path& dll) const;

    bool inject_and_await(const DWORD process_id, const std::filesystem::path& dll)
    {
        return this->inject(process_id, dll).await();
    }

    bool inject_and_await(utils::nt::null_handle process, const std::filesystem::path& dll)
    {
        return this->inject(std::move(process), dll).await();
    }

  private:
    uint32_t load_lib_rva_32{};
    uint32_t load_lib_rva_64{};
};
