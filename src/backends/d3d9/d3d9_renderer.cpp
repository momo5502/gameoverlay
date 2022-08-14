#include "std_include.hpp"
#include "d3d9_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::d3d9
{
	dimensions get_dimensions(IDirect3DDevice9* device)
	{
		D3DVIEWPORT9 viewport;
		device->GetViewport(&viewport);

		dimensions dimensions{};
		dimensions.width = uint32_t(viewport.Width);
		dimensions.height = uint32_t(viewport.Height);

		return dimensions;
	}

	renderer::renderer(IDirect3DDevice9* device)
		: device_(device)
	{
	}

	void renderer::draw_frame()
	{
		const auto current_dim = get_dimensions(this->device_);
		if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
		{
			this->canvas_ = std::make_unique<canvas>(this->device_, current_dim.width, current_dim.height);
		}

		this->on_frame(*this->canvas_);
		this->canvas_->draw();
	}
}

void gameoverlay::d3d9::renderer::reset()
{
	this->canvas_ = {};
}
