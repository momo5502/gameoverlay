#pragma once

#include <irenderer.hpp>
#include "hook_d3d9.hpp"
#include "hook_d3d9ex.hpp"

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

		void frame_callback(void* device);
		void reset_callback(void* device);

	private:
		hook_d3d9 d3d9_hook;
		hook_d3d9ex d3d9ex_hook;
		canvas canvas;

		void* device;

		std::function<void()> callback;
	};
}