#pragma once

#include <irenderer.hpp>
#include "d3d9_hook.hpp"
#include "d3d9ex_hook.hpp"

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

	private:
		d3d9_hook hook_d3d9;
		d3d9ex_hook hook_d3d9ex;
	};
}