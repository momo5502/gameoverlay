#include "overlay.hpp"

#include <list>
#include <d3d9_backend.hpp>

namespace gameoverlay
{
    namespace
    {
        struct overlay : utils::object
        {
            std::list<std::unique_ptr<backend>> backends{};

            overlay()
            {
                backends.emplace_back(d3d9::create_backend());
            }
        };
    }

    std::unique_ptr<utils::object> create_overlay()
    {
        return std::make_unique<overlay>();
    }
}
