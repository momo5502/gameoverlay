#pragma once

#include <irenderer.hpp>

namespace gameoverlay
{
	class renderer : public irenderer
	{
	public:
		renderer();
		virtual ~renderer() override;

		virtual bool is_available() override;

		virtual HWND get_window() override;
		virtual canvas* get_canvas() override;

		virtual void register_frame_callback(std::function<void()> callback) override;
	};
}