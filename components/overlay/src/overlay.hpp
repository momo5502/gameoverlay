#pragma once

#include "renderer_handler.hpp"

#include "cef/cef_ui.hpp"
#include "cef/cef_ui_app.hpp"
#include "cef/cef_ui_handler.hpp"

namespace gameoverlay
{

	class overlay
	{
	public:
		static void initialize();
		static void uninitialize();

	private:
		static std::unique_ptr<renderer_handler> handler;
		static std::unique_ptr<cef_ui> ui;
	};
}
