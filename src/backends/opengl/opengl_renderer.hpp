#pragma once

#include "../../renderer.hpp"
#include "opengl_canvas.hpp"

#include <memory>

namespace gameoverlay::opengl
{
	class renderer : public typed_renderer<backend_type::opengl>
	{
	public:
		renderer(const HDC hdc);
		void draw_frame();

	private:
		HDC hdc_{};
		std::unique_ptr<canvas> canvas_{};
	};
}
