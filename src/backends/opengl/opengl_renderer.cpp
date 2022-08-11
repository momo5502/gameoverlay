#include "std_include.hpp"
#include "opengl_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::opengl
{
	void initialize_glew()
	{
		static struct x
		{
			x()
			{
				if (glewInit() != GLEW_OK)
				{
					throw std::runtime_error("Failed to initialize glew");
				}
			}
		} _;
	}

	dimensions get_dimensions(const HDC hdc)
	{
		const HWND window = WindowFromDC(hdc);

		RECT rect;
		GetClientRect(window, &rect);

		dimensions dim{};
		dim.width = std::abs(rect.left - rect.right);
		dim.height = std::abs(rect.bottom - rect.top);
		return dim;
	}

	renderer::renderer(const HDC hdc)
		: hdc_(hdc)
	{
		initialize_glew();
	}

	void renderer::draw_frame()
	{
		const auto current_dim = get_dimensions(this->hdc_);
		if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
		{
			this->canvas_ = std::make_unique<canvas>(current_dim.width, current_dim.height);
		}

		this->on_frame(*this->canvas_);
		this->canvas_->draw();
	}
}
