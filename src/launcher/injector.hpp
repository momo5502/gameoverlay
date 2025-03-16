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

    operator bool() const
    {
        return this->thread_;
    }

    bool done()
    {
        return this->wait_for_thread(true);
    }

    bool await()
    {
        return this->wait_for_thread(false);
    }

  private:
    utils::nt::null_handle process_{};
    utils::nt::null_handle thread_{};
    void* memory_{};

    injection() = default;
    injection(utils::nt::null_handle process, utils::nt::null_handle thread, void* memory);

    bool release_memory();
    bool wait_for_thread(bool no_wait);
};

class injector
{
  public:
    injector();

    injection inject(DWORD process_id, const std::filesystem::path& dll) const;
    injection inject(utils::nt::null_handle process, const std::filesystem::path& dll) const;

  private:
    uint32_t load_lib_rva_32{};
    uint32_t load_lib_rva_64{};
};
