#pragma once

namespace gameoverlay
{
    enum class backend_type
    {
        unknown,
        d3d8,
        d3d9,
        d3d10,
        d3d11,
        d3d12,
        opengl,
        vulkan,
    };

    inline const char* get_backend_type_name(const backend_type type)
    {
        using enum backend_type;

        switch (type)
        {
        case d3d8:
            return "d3d8";
        case d3d9:
            return "d3d9";
        case d3d10:
            return "d3d10";
        case d3d11:
            return "d3d11";
        case d3d12:
            return "d3d12";
        case opengl:
            return "opengl";
        case vulkan:
            return "vulkan";

        case unknown:
        default:
            return "unknown";
        }
    }
}
