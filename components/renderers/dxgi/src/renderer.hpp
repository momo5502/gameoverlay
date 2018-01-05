#pragma once

#include <irenderer.hpp>
#include "dxgi_hook.hpp"

#include "canvas_dxgi.hpp"

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
		dxgi_hook hook_dxgi;
		canvas_dxgi dxgi_canvas;
		void* swap_chain;

		bool presented = false;
		std::function<void()> callback;
	};
}