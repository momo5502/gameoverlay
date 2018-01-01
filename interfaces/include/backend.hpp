#pragma once

#include "canvas.hpp"

namespace gameoverlay
{
	class backend
	{
	public:
		virtual ~backend() {};

		virtual bool is_available() = 0;

		virtual HWND get_window() = 0;
		virtual canvas* get_canvas() = 0;

		virtual void register_frame_callback(std::function<void()> callback) = 0;
	};

	typedef backend* __cdecl create_backend();
}
