#pragma once

#include <cstdint>
#include <span>
#include <filesystem>

namespace pe
{
    uint32_t find_export_rva(std::span<const uint8_t> pe_data, std::string_view export_name);
    uint32_t find_export_rva(const std::filesystem::path& file, std::string_view export_name);
}
