#pragma once

#include <utils/win.hpp>
#include <filesystem>

class injector
{
  public:
    injector();

    bool inject(DWORD process_id, const std::filesystem::path& dll);
    bool inject(HANDLE process, const std::filesystem::path& dll);

  private:
    uint32_t load_lib_rva_32{};
    uint32_t load_lib_rva_64{};
};
