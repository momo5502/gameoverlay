#include "std_include.hpp"

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

using namespace literally;

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
	dynlib self(instance);
	dynlib::add_load_path(self.get_folder() + "cef");
	dynlib libcef("libcef.dll", true);

	if (!libcef.is_valid() || !libcef.delay_import())
	{
		MessageBoxA(nullptr, "Unable to import libcef", "Error", MB_ICONERROR);
		return 1;
	}

	CefEnableHighDPISupport();

	CefMainArgs main_args(instance);
	return CefExecuteProcess(main_args, nullptr, nullptr);
}
