#pragma once

#include <memory>
#include <utils/class_helper.hpp>

namespace gameoverlay
{
    std::unique_ptr<utils::object> create_overlay();
}
