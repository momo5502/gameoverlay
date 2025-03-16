#include "pe.hpp"

#include <utils/io.hpp>
#include <utils/win.hpp>

namespace pe
{
    namespace
    {

        template <typename NtHeaders>
        uint32_t rva_to_file_offset(const NtHeaders& nt_headers, const uint32_t rva)
        {
            const auto* section = reinterpret_cast<const IMAGE_SECTION_HEADER*>(
                reinterpret_cast<const uint8_t*>(&nt_headers.OptionalHeader) +
                nt_headers.FileHeader.SizeOfOptionalHeader);

            for (uint16_t i = 0; i < nt_headers.FileHeader.NumberOfSections; ++i)
            {
                const auto section_rva = section[i].VirtualAddress;
                if (rva >= section_rva && rva < section_rva + section[i].Misc.VirtualSize)
                {
                    return rva - section_rva + section[i].PointerToRawData;
                }
            }

            return 0;
        }

        template <typename NtHeaders>
        uint32_t find_export_rva(const std::span<const uint8_t> pe_data, const std::string_view export_name)
        {
            const auto* data = pe_data.data();
            const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(data);
            const auto* nt_headers = reinterpret_cast<const NtHeaders*>(data + dos_header->e_lfanew);

            if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            {
                return 0;
            }

            const auto r2f = [&](const uint32_t rva) {
                return rva_to_file_offset(*nt_headers, rva); //
            };

            const auto& export_directory = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (export_directory.Size == 0)
            {
                return 0;
            }

            const auto export_dir_rva = r2f(export_directory.VirtualAddress);
            if (pe_data.size() < export_dir_rva + sizeof(IMAGE_EXPORT_DIRECTORY))
            {
                return 0;
            }

            const auto* export_dir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(data + export_dir_rva);

            const auto num_names = export_dir->NumberOfNames;
            const auto* name_rvas = reinterpret_cast<const DWORD*>(data + r2f(export_dir->AddressOfNames));
            const auto* ordinal_table = reinterpret_cast<const WORD*>(data + r2f(export_dir->AddressOfNameOrdinals));
            const auto* function_rvas = reinterpret_cast<const DWORD*>(data + r2f(export_dir->AddressOfFunctions));

            for (DWORD i = 0; i < num_names; ++i)
            {
                const std::string_view current_export_name = reinterpret_cast<const char*>(data + r2f(name_rvas[i]));

                if (current_export_name == export_name)
                {
                    const auto ordinal = ordinal_table[i];
                    return function_rvas[ordinal];
                }
            }

            return 0;
        }
    }

    uint32_t find_export_rva(const std::span<const uint8_t> pe_data, const std::string_view export_name)
    {
        const auto size = pe_data.size();
        const auto* data = pe_data.data();

        if (size < sizeof(IMAGE_DOS_HEADER))
        {
            return 0;
        }

        const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(data);

        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE ||
            size < dos_header->e_lfanew + std::max(sizeof(IMAGE_NT_HEADERS64), sizeof(IMAGE_NT_HEADERS32)))
        {
            return 0;
        }

        const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(data + dos_header->e_lfanew);

        if (nt_headers->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            return find_export_rva<IMAGE_NT_HEADERS32>(pe_data, export_name);
        }

        if (nt_headers->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            return find_export_rva<IMAGE_NT_HEADERS64>(pe_data, export_name);
        }

        return 0;
    }

    uint32_t find_export_rva(const std::filesystem::path& file, const std::string_view export_name)
    {
        std::string data{};
        if (!utils::io::read_file(file, &data))
        {
            return 0;
        }

        const auto binary_data = std::span(reinterpret_cast<const uint8_t*>(data.data()), data.size());
        return find_export_rva(binary_data, export_name);
    }
}
