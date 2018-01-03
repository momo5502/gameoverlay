#pragma once

#include <irenderer.hpp>
#include "dxgi_hook.hpp"

#include "canvas_d3d11.hpp"

namespace gameoverlay
{
	class renderer : public irenderer
	{
	public:
		renderer();
		virtual ~renderer() override;

		virtual bool is_available() override;

		virtual HWND get_window() override;
		virtual icanvas* get_canvas() override;

		virtual void register_frame_callback(std::function<void()> callback) override;
		virtual void unregister_frame_callback() override;

		void frame_callback(void* swap_chain);

	private:
		dxgi_hook hook;
		canvas_d3d11 d3d11_canvas;

		bool presented = false;
		std::function<void()> callback;
	};
}