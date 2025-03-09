#pragma once

#include "d3d8_win.hpp"

#include <stdexcept>
#include <utils/class_helper.hpp>

namespace gameoverlay::d3d8
{
    namespace detail
    {
        using owned_state_block = utils::owned_object<DWORD>;

        inline owned_state_block create_state_block(IDirect3DDevice8& device)
        {
            DWORD block{};
            if (FAILED(device.CreateStateBlock(D3DSBT_ALL, &block)))
            {
                throw std::runtime_error("Failed to create state block");
            }

            CComPtr dev{&device};

            return {block, [dev](const DWORD b) {
                        dev->DeleteStateBlock(b); //
                    }};
        }

        inline HRESULT set_fvf(IDirect3DDevice8& device, const DWORD handle)
        {
            return device.SetVertexShader(handle);
        }

        inline CComPtr<IDirect3DTexture8> create_texture(IDirect3DDevice8& device, const UINT width, const UINT height,
                                                         const UINT levels, const DWORD usage, const D3DFORMAT format,
                                                         const D3DPOOL pool)
        {
            CComPtr<IDirect3DTexture8> texture{};
            if (FAILED(device.CreateTexture(width, height, levels, usage, format, pool, &texture)))
            {
                throw std::runtime_error("Failed to create texture");
            }

            return texture;
        }
    }

    struct d3d8_scope
    {
        CComPtr<IDirect3DDevice8> device{};
        detail::owned_state_block state_block{};
        detail::owned_state_block empty_state_block{};

        d3d8_scope(IDirect3DDevice8& dev)
            : device(&dev),
              state_block(detail::create_state_block(dev)),
              empty_state_block(detail::create_state_block(dev))
        {
        }

        void enter() const
        {
            this->device->CaptureStateBlock(this->state_block);
            this->device->ApplyStateBlock(this->empty_state_block);
        }

        void leave() const
        {
            this->device->ApplyStateBlock(this->state_block);
        }
    };

    struct d3d8_traits
    {
        using device = IDirect3DDevice8;
        using texture2d = IDirect3DTexture8;
        using scope = d3d8_scope;

        using viewport = D3DVIEWPORT8;

        static constexpr auto set_fvf_func = detail::set_fvf;
        static constexpr auto create_texture_func = detail::create_texture;
    };
}
