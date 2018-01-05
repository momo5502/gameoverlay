#pragma once

#include "icanvas.hpp"
#include "canvas_d3d10.hpp"
#include "canvas_d3d11.hpp"

namespace gameoverlay
{
	class canvas_dxgi : public icanvas
	{
	public:
		virtual ~canvas_dxgi() override;

		virtual bool is_available() override;

		virtual uint32_t get_width() override;
		virtual uint32_t get_height() override;

		virtual bool paint(const void* buffer) override;

		void draw(IDXGISwapChain* swap_chain);

	private:
		canvas_d3d10 d3d10_canvas;
		canvas_d3d11 d3d11_canvas;

		bool is_d3d10_available();
		bool is_d3d11_available();

		void draw_d3d10(ID3D10Device* device);
		void draw_d3d11(ID3D11Device* device);
	};
}
