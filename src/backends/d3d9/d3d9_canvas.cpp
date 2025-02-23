#include "d3d9_canvas.hpp"

#include <cassert>
#include <stdexcept>

#include "utils/finally.hpp"

namespace gameoverlay::d3d9
{
    canvas::canvas(IDirect3DDevice9* device, const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height),
          device_(device)
    {
        if (FAILED(D3DXCreateSprite(this->device_, &this->sprite_)))
        {
            throw std::runtime_error("Failed to create sprite");
        }

        if (FAILED(this->device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                                &this->texture_, nullptr)))
        {
            throw std::runtime_error("Failed to create texture");
        }
    }

    void canvas::paint(const void* image)
    {
        D3DSURFACE_DESC desc{};
        this->texture_->GetLevelDesc(0, &desc);

        if ((desc.Usage & D3DUSAGE_DYNAMIC) != D3DUSAGE_DYNAMIC)
        {
            return;
        }

        D3DLOCKED_RECT locked_rect;
        if (FAILED(this->texture_->LockRect(0, &locked_rect, NULL, 0)))
        {
            return;
        }

        const auto _ = utils::finally([&] {
            this->texture_->UnlockRect(0); //
        });

        const auto bytes_per_pixel = static_cast<uint32_t>(locked_rect.Pitch) / this->get_width();
        assert(bytes_per_pixel == 4);

        memcpy(locked_rect.pBits, image, this->get_width() * this->get_height() * bytes_per_pixel);
    }

    void canvas::draw() const
    {
        this->device_->BeginScene();

        const D3DXVECTOR3 position(0.0, 0.0, 0.0f);

        this->sprite_->Begin(D3DXSPRITE_ALPHABLEND);
        this->sprite_->Draw(this->texture_, nullptr, nullptr, &position, 0xFFFFFFFF);
        this->sprite_->End();

        this->device_->EndScene();
    }
}
