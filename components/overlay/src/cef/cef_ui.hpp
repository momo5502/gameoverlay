#pragma once

#pragma warning(push)
#pragma warning(disable: 4100)

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_command_line.h"
#include "include/cef_frame.h"
#include "include/cef_web_plugin.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#pragma warning(pop)

#include <literally/library.hpp>

namespace gameoverlay
{
	class cef_ui_app;

	class cef_ui
	{
	public:
		cef_ui();
		~cef_ui();

		CefRefPtr<cef_ui_app> get_app();

	private:
		std::string path;
		std::thread ui_thread;
		CefRefPtr<cef_ui_app> app;

		void ui_runner();
	};
}
