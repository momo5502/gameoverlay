#pragma once

#include "d3d9_win.hpp"

#include <stdexcept>
#include <utils/class_helper.hpp>

namespace gameoverlay::d3d9
{
    namespace detail
    {
        inline CComPtr<IDirect3DStateBlock9> create_state_block(IDirect3DDevice9& device)
        {
            CComPtr<IDirect3DStateBlock9> block{};
            if (FAILED(device.CreateStateBlock(D3DSBT_ALL, &block)))
            {
                throw std::runtime_error("Failed to create state block");
            }

            return block;
        }

        inline HRESULT set_fvf(IDirect3DDevice9& device, const DWORD handle)
        {
            return device.SetFVF(handle);
        }

        inline CComPtr<IDirect3DTexture9> create_texture(IDirect3DDevice9& device, const UINT width, const UINT height,
                                                         const UINT levels, const DWORD usage, const D3DFORMAT format,
                                                         const D3DPOOL pool)
        {
            CComPtr<IDirect3DTexture9> texture{};
            if (FAILED(device.CreateTexture(width, height, levels, usage, format, pool, &texture, nullptr)))
            {
                throw std::runtime_error("Failed to create texture");
            }

            return texture;
        }
    }

    struct d3d9_scope
    {
        CComPtr<IDirect3DStateBlock9> state_block{};
        CComPtr<IDirect3DStateBlock9> empty_state_block{};

        d3d9_scope(IDirect3DDevice9& dev)
            : state_block(detail::create_state_block(dev)),
              empty_state_block(detail::create_state_block(dev))
        {
        }

        void enter() const
        {
            this->state_block->Capture();
            this->empty_state_block->Apply();
        }

        void leave() const
        {
            this->state_block->Apply();
        }
    };

    struct d3d9_traits
    {
        using device = IDirect3DDevice9;
        using texture2d = IDirect3DTexture9;
        using scope = d3d9_scope;

        using viewport = D3DVIEWPORT9;

        static constexpr auto set_fvf_func = detail::set_fvf;
        static constexpr auto create_texture_func = detail::create_texture;
    };
}
