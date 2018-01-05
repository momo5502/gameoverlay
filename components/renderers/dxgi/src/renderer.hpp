#pragma once

#include <irenderer.hpp>
#include "hook_dxgi.hpp"

#include "canvas.hpp"

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
		hook_dxgi dxgi_hook;
		canvas canvas;
		void* swap_chain;

		bool presented = false;
		std::function<void()> callback;
	};
}