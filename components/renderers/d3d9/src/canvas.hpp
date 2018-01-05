#pragma once

#include "icanvas.hpp"
#include "canvas_d3d9.hpp"

namespace gameoverlay
{
	class canvas : public icanvas
	{
	public:
		virtual ~canvas() override;

		virtual bool is_available() override;

		virtual uint32_t get_width() override;
		virtual uint32_t get_height() override;

		virtual bool paint(const void* buffer) override;

		void draw(IDirect3DDevice9* device);
		void reset();

	private:
		canvas_d3d9 d3d9_canvas;
	};
}
