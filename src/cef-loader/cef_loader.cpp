#include "cef_loader.hpp"

#include <utils/nt.hpp>

namespace cef_loader
{
    namespace
    {
        std::filesystem::path get_own_directory()
        {
            const auto self = utils::nt::library::get_by_address(&get_own_directory);
            return self.get_folder();
        }

        void load_cef_library()
        {
            const auto cef_path = get_cef_path();
            const auto cef_lib = cef_path / "libcef.dll";

            const auto cookie = AddDllDirectory(cef_path.wstring().c_str());
            const auto library = LoadLibraryExW(cef_lib.wstring().c_str(), nullptr,
                                                LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 |
                                                    LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

            RemoveDllDirectory(cookie);

            if (!library)
            {
                throw std::runtime_error("Failed to load cef");
            }
        }
    }

    std::filesystem::path get_cef_path()
    {
        return get_own_directory() / "cef";
    }

    void load_cef()
    {
        static const auto x = [] {
            load_cef_library();
            return 0;
        }();
        (void)x;
    }
}
