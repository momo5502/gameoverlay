#include "std_include.hpp"
#include "canvas.hpp"

namespace gameoverlay
{
	canvas::~canvas()
	{

	}

	bool canvas::is_available()
	{
		return this->d3d9_canvas.is_loaded();
	}

	uint32_t canvas::get_width()
	{
		if (this->is_available()) return this->d3d9_canvas.width;
		return 0;
	}

	uint32_t canvas::get_height()
	{
		if (this->is_available()) return this->d3d9_canvas.height;
		return 0;
	}

	bool canvas::paint(const void* buffer)
	{
		// TODO: Do channel swapping here
		if (this->is_available()) return this->d3d9_canvas.update(buffer);
		return false;
	}

	void canvas::draw(IDirect3DDevice9* device)
	{
		if (!device) return;

		D3DVIEWPORT9 viewport;
		device->GetViewport(&viewport);

		uint32_t new_width = uint32_t(viewport.Width);
		uint32_t new_height = uint32_t(viewport.Height);

		if (this->d3d9_canvas.get_device() != device)
		{
			this->d3d9_canvas.release_resources();
		}

		if (!this->d3d9_canvas.is_initialized())
		{
			this->d3d9_canvas.initialize(device);
			this->d3d9_canvas.create(new_width, new_height, D3DFMT_A8R8G8B8);
		}

		if (this->d3d9_canvas.is_initialized())
		{
			if (this->d3d9_canvas.width != new_width || this->d3d9_canvas.height != new_height)
			{
				this->d3d9_canvas.resize(new_width, new_height);
			}

			// TODO: Creating a new device won't properly release the resources, as the pointer doesn't change :/
			// Checking if drawing succeeded doesn't seem to work :(
			if (!this->d3d9_canvas.draw(0, 0))
			{
				this->d3d9_canvas.release_resources();
			}
		}
	}

	void canvas::reset()
	{
		this->d3d9_canvas.release_resources();
	}
}
