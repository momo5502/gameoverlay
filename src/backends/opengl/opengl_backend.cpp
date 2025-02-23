#include "opengl_backend.hpp"

#include <backend_opengl.hpp>
#include <utils/hook.hpp>

namespace gameoverlay::opengl
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour swap_buffers{};
            utils::hook::detour delete_dc{};
        };

        hooks& get_hooks()
        {
            static hooks h{};
            return h;
        }

        // TODO: Synchronize access with object destruction
        opengl_backend* g_backend{nullptr};

        void draw_frame(const HDC hdc)
        {
            if (!hdc || !g_backend)
            {
                return;
            }

            g_backend->create_or_access_renderer(
                hdc,
                [](opengl_renderer& r) {
                    r.draw_frame(); //
                },
                hdc);
        }

        BOOL WINAPI swap_buffers_stub(const HDC hdc)
        {
            draw_frame(hdc);
            return get_hooks().swap_buffers.invoke_stdcall<BOOL>(hdc);
        }

        BOOL WINAPI delete_dc_stub(const HDC hdc)
        {
            if (g_backend)
            {
                g_backend->erase(hdc);
            }

            return get_hooks().delete_dc.invoke_stdcall<BOOL>(hdc);
        }
    }

    opengl_backend::~opengl_backend()
    {
        g_backend = nullptr;
    }

    opengl_backend::opengl_backend(owned_handler h)
        : typed_backed(std::move(h))
    {
        g_backend = this;

        auto& hooks = get_hooks();

        hooks.swap_buffers.create(::SwapBuffers, swap_buffers_stub);
        hooks.delete_dc.create(::DeleteDC, delete_dc_stub);
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<opengl_backend>(std::move(h));
    }
}
