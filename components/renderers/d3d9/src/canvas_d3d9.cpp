#include "std_include.hpp"
#include "canvas_d3d9.hpp"

namespace gameoverlay
{
	canvas_d3d9::canvas_d3d9()
	{
		this->reset_resources();
	}

	canvas_d3d9::canvas_d3d9(IDirect3DDevice9* _device) : canvas_d3d9()
	{
		this->initialize(_device);
	}

	canvas_d3d9::~canvas_d3d9()
	{
		this->release_resources();
	}

	IDirect3DDevice9* canvas_d3d9::get_device()
	{
		return this->device;
	}

	void canvas_d3d9::release_resources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		release_resource(this->texture);
		release_resource(this->sprite);
		release_resource(this->device);

		this->free_buffer();
	}

	void canvas_d3d9::reset_resources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->bytes_per_pixel = 0;
		this->requires_update = false;

		reset_resource(this->texture);
		reset_resource(this->sprite);
		reset_resource(this->device);
		reset_resource(this->buffer);

		this->format = D3DFMT_UNKNOWN;
	}

	bool canvas_d3d9::create(std::string file)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release old data
		release_resource(this->texture);

		if (FAILED(D3DXCreateTextureFromFileExA(this->device, file.c_str(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &this->texture)) || !this->texture)
		{
			release_resource(this->texture);
			return false;
		}

		D3DSURFACE_DESC desc;
		this->texture->GetLevelDesc(0, &desc);

		this->width = desc.Width;
		this->height = desc.Height;

		return true;
	}

	bool canvas_d3d9::create(uint32_t _width, uint32_t _height, D3DFORMAT _format, const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release old data
		release_resource(this->texture);
		if (!this->device) return false;

		if (this->device->CreateTexture(_width, _height, 1, D3DUSAGE_DYNAMIC, _format, D3DPOOL_DEFAULT, &this->texture, NULL) != D3D_OK || !this->texture)
		{
			release_resource(this->texture);
			return false;
		}

		this->width = _width;
		this->height = _height;
		this->format = _format;

		D3DLOCKED_RECT locked_rect;
		if (SUCCEEDED(this->texture->LockRect(0, &locked_rect, NULL, 0)))
		{
			this->bytes_per_pixel = (locked_rect.Pitch / this->width);
			this->texture->UnlockRect(0);
		}

		this->free_buffer();
		this->buffer = new char[this->width * this->height * this->bytes_per_pixel];

		return this->perform_update(_buffer);
	}

	bool canvas_d3d9::resize(uint32_t _width, uint32_t _height)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		bool success = false;

		if (this->width != _width || this->height != _height)
		{
			if (this->texture)
			{
				D3DSURFACE_DESC desc;
				this->texture->GetLevelDesc(0, &desc);

				if ((desc.Usage & D3DUSAGE_DYNAMIC) == D3DUSAGE_DYNAMIC)
				{
					success = this->create(_width, _height, desc.Format);
				}
			}
		}

		return success;
	}

	bool canvas_d3d9::update(const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (_buffer)
		{
			std::memcpy(this->buffer, _buffer, this->width * this->height * this->bytes_per_pixel);
			this->requires_update = true;
		}

		return true;
	}

	bool canvas_d3d9::update(std::function<void(void*, uint32_t, uint32_t, uint32_t)> callback)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (callback)
		{
			callback(this->buffer, this->width, this->height, this->bytes_per_pixel);
			this->requires_update = true;
		}

		return true;
	}

	bool canvas_d3d9::perform_update(const void* _buffer)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		bool success = false;

		// Check texture and buffer validity
		if (_buffer && this->texture)
		{
			D3DSURFACE_DESC desc;
			this->texture->GetLevelDesc(0, &desc);

			// Check if updating is allowed
			if ((desc.Usage & D3DUSAGE_DYNAMIC) == D3DUSAGE_DYNAMIC)
			{
				// Map texture buffer
				D3DLOCKED_RECT locked_rect;
				if (SUCCEEDED(this->texture->LockRect(0, &locked_rect, NULL, 0)))
				{
					// Copy new data into the buffer
					int bbp = (int)(locked_rect.Pitch / this->width);
					int bpr = this->width * bbp;

					for (uint32_t i = 0; i < this->height; i++)
					{
						memcpy((char*)locked_rect.pBits + (i * locked_rect.Pitch), LPSTR(_buffer) + i * bpr, bpr);
					}

					// Unmap texture
					this->texture->UnlockRect(0);

					success = true;
				}
			}
		}
		return success;
	}

	void canvas_d3d9::free_buffer()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (this->buffer)
		{
			delete[] this->buffer;
			this->buffer = nullptr;
		}

		this->requires_update = false;
	}

	bool canvas_d3d9::initialize(IDirect3DDevice9* _device)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->release_resources();

		if (!_device) return false;

		this->device = _device;
		this->device->AddRef();

		return this->create_resources();
	}

	void canvas_d3d9::reinitialize(IDirect3DDevice9* pDevice)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		this->initialize(pDevice);
		this->create(this->width, this->height, this->format);
	}

	bool canvas_d3d9::create_resources()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		// Release used resources
		release_resource(this->sprite);

		if (FAILED(D3DXCreateSprite(this->device, &this->sprite)) || !this->sprite)
		{
			release_resource(this->sprite);
			return false;
		}

		return true;
	}

	bool canvas_d3d9::draw(int32_t x, int32_t y, COLORREF color)
	{
		return this->draw(x, y, this->width, this->height, color);
	}

	// Width and height not needed yet
	bool canvas_d3d9::draw(int32_t x, int32_t y, uint32_t /*width*/, uint32_t /*height*/, COLORREF color)
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);

		if (this->requires_update)
		{
			this->perform_update(this->buffer);
			this->requires_update = false;
		}

		D3DXVECTOR3 position((float)x, (float)y, 0.0f);

		bool result = false;
		if (this->sprite && this->texture)
		{
			this->device->BeginScene();

			this->sprite->Begin(D3DXSPRITE_ALPHABLEND);
			result = SUCCEEDED(this->sprite->Draw(this->texture, NULL, NULL, &position, color));
			this->sprite->End();

			this->device->EndScene();
		}

		return result;
	}

	bool canvas_d3d9::is_initialized()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		return (this->device != NULL);
	}

	bool canvas_d3d9::is_loaded()
	{
		std::lock_guard<std::recursive_mutex> _(this->mutex);
		return (this->texture != NULL);
	}
}
