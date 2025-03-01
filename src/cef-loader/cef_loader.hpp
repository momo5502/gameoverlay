#pragma once
#include <filesystem>

namespace cef_loader
{
    std::filesystem::path get_cef_path();
    void load_cef();
}
