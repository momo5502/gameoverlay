#pragma once

#include "../../renderer.hpp"
#include "d3d9_canvas.hpp"

#include <memory>

namespace gameoverlay::d3d9
{
	class renderer : public window_renderer<backend_type::d3d9>
	{
	public:
		renderer(IDirect3DDevice9* device);
		void draw_frame();
		void reset();

	private:
		IDirect3DDevice9* device_{};
		std::unique_ptr<canvas> canvas_{};
	};
}
